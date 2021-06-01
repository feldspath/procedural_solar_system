#include <iostream>
#include <time.h>
#include <vector>
#include <thread>

#include "vcl/vcl.hpp"
#include "planet.hpp"
#include "physics.hpp"
#include "skybox.hpp"
#include "camera_fps.hpp"
#include "player.hpp"
#include "vegetation.hpp"
#include "display.hpp"

using namespace vcl;

struct keyboard_state_parameters {
	int right = 0;
	int front = 0;
	int up = 0;
};

struct user_interaction_parameters {
	vec2 mouse_prev;
	timer_fps fps_record;
	bool cursor_on_gui;
	keyboard_state_parameters keyboard_state;
};
user_interaction_parameters user;

scene_environment scene;

void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
void window_size_callback(GLFWwindow* window, int width, int height);

void initialize_data();

float current_width;
float current_height;

int planet_index;

int main(int, char* argv[])
{
	std::cout << "Run " << argv[0] << std::endl;
	GLFWwindow* window = create_window(SCR_WIDTH, SCR_HEIGHT);
	window_size_callback(window, SCR_WIDTH, SCR_HEIGHT);
	std::cout << opengl_info_display() << std::endl;

	imgui_init(window);
	glfwSetKeyCallback(window, keyboard_callback);
	#if CAMERA_TYPE
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	#endif
	glfwSetCursorPosCallback(window, mouse_move_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);
	
	std::cout<<"Initialize data ..."<<std::endl;
	initialize_data();


	std::cout<<"Start animation loop ..."<<std::endl;
	user.fps_record.start();
	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window))
	{
		float deltaTime = user.fps_record.update();

		// Camera 
		#if CAMERA_TYPE
			int3 direction = { user.keyboard_state.front, user.keyboard_state.right, user.keyboard_state.up };
			scene.player.update_position(direction);
		#else
			scene.camera.center_of_rotation = scene.planets[planet_index].getPosition();
			//scene.light = scene.camera.position();
		#endif
		
		// Physics
		PhysicsComponent::update(deltaTime);
		for (int i = 0; i < scene.planets.size(); i++)
			scene.planets[i].updateRotation(deltaTime);
		
		imgui_create_frame();
		if(user.fps_record.event) {
			std::string const title = "VCL Display - "+str(user.fps_record.fps)+" fps";
			glfwSetWindowTitle(window, title.c_str());
		}

#if !CAMERA_TYPE
		ImGui::Begin("GUI",NULL,ImGuiWindowFlags_AlwaysAutoResize);
		user.cursor_on_gui = ImGui::IsAnyWindowFocused();	
#endif

		display_scene(scene, user.fps_record.t, current_width, current_height);

		// Clean buffers
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		

#if !CAMERA_TYPE
		display_interface(scene, &planet_index);
		ImGui::End();
		imgui_render_frame(window);
#endif

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	vcl::imgui_cleanup();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}


void initialize_data()
{
	// SHADERS
	GLuint const shader_mesh = opengl_create_shader_program(opengl_shader_preset("mesh_vertex"), opengl_shader_preset("mesh_fragment"));
	GLuint const shader_uniform_color = opengl_create_shader_program(opengl_shader_preset("single_color_vertex"), opengl_shader_preset("single_color_fragment"));
	GLuint const texture_white = opengl_texture_to_gpu(image_raw{1,1,image_color_type::rgba,{255,255,255,255}});
	mesh_drawable::default_shader = shader_mesh;
	mesh_drawable::default_texture = texture_white;
	curve_drawable::default_shader = shader_uniform_color;
	segments_drawable::default_shader = shader_uniform_color;	

	// CAMERA
	#if CAMERA_TYPE
		scene.camera.position_camera = { 0, 0, 0};
		scene.camera.orientation_camera = rotation();
		scene.player.bind_camera(&scene.camera);
		scene.player.bind_planets(&scene.planets);
		PhysicsComponent::player = &scene.player;
	#else
	#endif

	// SKYBOX
	scene.skybox = Skybox("assets/cubemap.png");

	// PLANETS INITIALIZER
    Planet::initPlanetRenderer(SCR_WIDTH, SCR_HEIGHT);
	int nPlanets = 9;
	scene.planets.resize(nPlanets);
	// Sun
	float sun_mass = (float)1e16;
	scene.planets[0] = Planet("HeliosStar", sun_mass, { -2000, 50, 0 }, {0, 0, 0}, 50);
	scene.planets[0].isSun = true;

	// Other planets
	scene.planets[1] = Planet("Vulkan", 0.13 / PhysicsComponent::G * 90000, &(scene.planets[0]), 1000.0f, rand_interval(0.0f, 2 * 3.14f), resolution);
	scene.planets[1].waterGlow = true;

	scene.planets[2] = Planet("Gwen", 0.1 / PhysicsComponent::G * 250000, &(scene.planets[0]), 2000.0f, 0.0f, resolution);
	scene.planets[2].specularWater = false;

	scene.planets[3] = Planet("Companion", 0.07 / PhysicsComponent::G * 22500, &(scene.planets[2]), 200.0f, 3.14f/2, resolution / 2);

	scene.planets[4] = Planet("Oculus", 0.2 / PhysicsComponent::G * 1000000, &(scene.planets[0]), 3500.0f, rand_interval(0.0f, 2 * 3.14f), 50);
	scene.planets[4].specularWater = false;
	scene.planets[5] = Planet("Scylla", 0.07 / PhysicsComponent::G * 40000, &(scene.planets[4]), 300.0f, rand_interval(0.0f, 2 * 3.14f), resolution);

	scene.planets[6] = Planet("AethedisPrime", 0.1 / PhysicsComponent::G * 90000, &(scene.planets[0]), 5000.0f, rand_interval(0.0f, 2 * 3.14f), resolution);
	scene.planets[6].visual.shading.phong.specular = 0.3f;
	scene.planets[6].visualLowRes.shading.phong.specular = 0.3f;

	float dualMass = 0.07 / PhysicsComponent::G * 40000;
	float separation = 130.0f;
	float sunDistance = 7000.0f;
	vcl::vec3 position1 = scene.planets[0].getPosition() + sunDistance * vcl::vec3(1.0f, 0.0f, 0.0f);
	vcl::vec3 position2 = scene.planets[0].getPosition() + (sunDistance - separation) * vcl::vec3(1.0f, 0.0f, 0.0f);
	vcl::vec3 relativeVelocity = std::sqrt(PhysicsComponent::G * dualMass / (2 * separation)) * vcl::vec3(0.0f, 1.0f, 0.0f);
	vcl::vec3 velocity = std::sqrt(PhysicsComponent::G * sun_mass / sunDistance) * vcl::vec3(0.0f, 1.0f, 0.0f) + scene.planets[0].getSpeed();

	scene.planets[7] = Planet("M458", dualMass, position1, relativeVelocity + velocity, resolution);
	scene.planets[8] = Planet("EE", dualMass, position2, -relativeVelocity + velocity, resolution);

	planet_index = 2;

	// Create the meshes of the planets in parallel
	std::vector<std::thread> threads(nPlanets);
	for (int i = 0; i < nPlanets; i++) {
		threads[i] = std::thread(&Planet::updatePlanetMesh, &scene.planets[i]);
	}
	for (int i = 0; i < nPlanets; i++) {
		threads[i].join();
	}
	for (int i = 0; i < nPlanets; i++) {
		scene.planets[i].updateVisual();
	}

	// Create the plants mesh and spawn parameters
	createPlant(scene.plant);
	scene.plantInfos = plantSpawn(5000, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), 0.5);
}


// CALLBACK FUNCTIONS
void window_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	buildFrustrsums(scene, width, height);
	Planet::buildTextures(width, height);
	current_width = width;
	current_height = height;
}

void mouse_move_callback(GLFWwindow* window, double xpos, double ypos) {
	vec2 const  p1 = glfw_get_mouse_cursor(window, xpos, ypos);
	vec2 const& p0 = user.mouse_prev;
	glfw_state state = glfw_current_state(window);

	auto& camera = scene.camera;
	if(!user.cursor_on_gui){
		#if CAMERA_TYPE
			vec2 offset = p1 - p0;
			offset.x = -offset.x;
			camera.update_orientation(offset);
		#else
			if (state.mouse_click_left && !state.key_ctrl)
				camera.manipulator_rotate_trackball(p0, p1);
			if (state.mouse_click_left && state.key_ctrl)
				camera.manipulator_translate_in_plane(p1 - p0);
			if (state.mouse_click_right)
				camera.manipulator_scale_distance_to_center((p1 - p0).y);
		#endif

		
	}
	user.mouse_prev = p1;
}

void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_W) {
		if (user.keyboard_state.front < 1 && (action == GLFW_PRESS))
			user.keyboard_state.front++;
		if (user.keyboard_state.front > -1 && (action == GLFW_RELEASE))
			user.keyboard_state.front--;
	}
	if (key == GLFW_KEY_S) {
		if (user.keyboard_state.front > -1 && (action == GLFW_PRESS))
			user.keyboard_state.front--;
		if (user.keyboard_state.front < 1 && (action == GLFW_RELEASE))
			user.keyboard_state.front++;
	}
	if (key == GLFW_KEY_D) {
		if (user.keyboard_state.right < 1 && (action == GLFW_PRESS))
			user.keyboard_state.right++;
		if (user.keyboard_state.right > -1 && (action == GLFW_RELEASE))
			user.keyboard_state.right--;
	}
	if (key == GLFW_KEY_A) {
		if (user.keyboard_state.right > -1 && (action == GLFW_PRESS))
			user.keyboard_state.right--;
		if (user.keyboard_state.right < 1 && (action == GLFW_RELEASE))
			user.keyboard_state.right++;
	}
	if (key == GLFW_KEY_LEFT_SHIFT) {
		if (user.keyboard_state.up < 1 && (action == GLFW_PRESS))
			user.keyboard_state.up++;
		if (user.keyboard_state.up > -1 && (action == GLFW_RELEASE))
			user.keyboard_state.up--;
	}
	if (key == GLFW_KEY_LEFT_CONTROL) {
		if (user.keyboard_state.up > -1 && (action == GLFW_PRESS))
			user.keyboard_state.up--;
		if (user.keyboard_state.up < 1 && (action == GLFW_RELEASE))
			user.keyboard_state.up++;
	}
	if (key == GLFW_KEY_E && action == GLFW_PRESS) {
		scene.player.toggleJetpack();
	}
}
