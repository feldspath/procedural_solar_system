#include "vcl/vcl.hpp"
#include <iostream>
#include <time.h>
#include <vector>

#include "planet.hpp"
#include "physics.hpp"
#include "skybox.hpp"
#include "camera_fps.hpp"
#include "player.hpp"

// 0 is edit mode
// 1 is explore mode	
#define CAMERA_TYPE 0

using namespace vcl;

struct gui_parameters {
	bool display_frame = false;
	bool display_wireframe = false;
};

struct keyboard_state_parameters {
	int right = 0;
	int front = 0;
	int up = 0;

};

struct user_interaction_parameters {
	vec2 mouse_prev;
	timer_fps fps_record;
	mesh_drawable global_frame;
	gui_parameters gui;
	bool cursor_on_gui;
	keyboard_state_parameters keyboard_state;
};
user_interaction_parameters user;

#if CAMERA_TYPE
struct scene_environment
{
	camera_fps camera;
	mat4 projection;
	mat4 projection;
	mat4 farProjection;
	vec3 light;
	Player player;
	std::vector<Planet> planets;
	Skybox skybox;
};
#else
struct scene_environment
{
	camera_around_center camera;
	mat4 projection;
	mat4 farProjection;
	mat4 nearProjection;
	vec3 light;
	std::vector<Planet> planets;
	Skybox skybox;
};
#endif

scene_environment scene;

void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
void window_size_callback(GLFWwindow* window, int width, int height);

void initialize_data();
void display_scene();
void display_interface();

unsigned int SCR_WIDTH = 1280;
unsigned int SCR_HEIGHT = 1024;

float previousTime;
float deltaTime;

int planet_index;

int main(int, char* argv[])
{
	std::cout << "Run " << argv[0] << std::endl;
	int const width = SCR_WIDTH;
	int const height = SCR_HEIGHT;
	GLFWwindow* window = create_window(width, height);
	window_size_callback(window, width, height);
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
	previousTime = glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{

		//Sleep(20);
		deltaTime = glfwGetTime() - previousTime;
		previousTime = glfwGetTime();


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
		
		user.fps_record.update();
			

		imgui_create_frame();
		if(user.fps_record.event) {
			std::string const title = "VCL Display - "+str(user.fps_record.fps)+" fps";
			glfwSetWindowTitle(window, title.c_str());
		}

		ImGui::Begin("GUI",NULL,ImGuiWindowFlags_AlwaysAutoResize);
		user.cursor_on_gui = ImGui::IsAnyWindowFocused();
		
		if(user.gui.display_frame) draw(user.global_frame, scene);

		display_scene();

		// Clean buffers
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		
		display_interface();

		ImGui::End();
		imgui_render_frame(window);
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
	GLuint const shader_mesh = opengl_create_shader_program(opengl_shader_preset("mesh_vertex"), opengl_shader_preset("mesh_fragment"));
	GLuint const shader_uniform_color = opengl_create_shader_program(opengl_shader_preset("single_color_vertex"), opengl_shader_preset("single_color_fragment"));
	GLuint const texture_white = opengl_texture_to_gpu(image_raw{1,1,image_color_type::rgba,{255,255,255,255}});
	mesh_drawable::default_shader = shader_mesh;
	mesh_drawable::default_texture = texture_white;
	curve_drawable::default_shader = shader_uniform_color;
	segments_drawable::default_shader = shader_uniform_color;	
	
	user.global_frame = mesh_drawable(mesh_primitive_frame());
	user.gui.display_frame = false;

	float sun_mass = (float)3e16;

	#if CAMERA_TYPE
		scene.camera.position_camera = { 5000, 5, 10};
		scene.camera.orientation_camera = rotation();
		scene.player.bind_camera(&scene.camera);
		scene.player.init_physics();
		scene.player.bind_planets(&scene.planets);
		PhysicsComponent::player = &scene.player;
	#else
	#endif

	scene.skybox = Skybox("assets/cubemap.png");

    Planet::initPlanetRenderer(SCR_WIDTH, SCR_HEIGHT);
	scene.planets.resize(6);
	// Sun
	scene.planets[0] = Planet("HeliosStar", sun_mass, { 0, 0, 0 }, {0, 0, 0}, 50);
	scene.planets[0].isSun = true;
	scene.light = { 0.0f, 0.0f, 0.0f };

	// Other planets
	scene.planets[1] = Planet("Vulkan", 0.1 / PhysicsComponent::G * 900, &(scene.planets[0]), 1000.0f, rand_interval(0.0f, 2*3.14f), 200);
	scene.planets[1].waterGlow = true;

	scene.planets[2] = Planet("Oculus", 0.2 / PhysicsComponent::G * 10000, &(scene.planets[0]), 3500.0f, rand_interval(0.0f, 2 * 3.14f), 200);
	scene.planets[2].waterLevel /= 10.0f;
	scene.planets[2].atmosphereHeight /= 10.0f;
	scene.planets[3] = Planet("Scylla", 0.07 / PhysicsComponent::G * 400, &(scene.planets[2]), 300.0f, rand_interval(0.0f, 2 * 3.14f), 200);

	scene.planets[4] = Planet("AethedisPrime", 0.1 / PhysicsComponent::G * 900, &(scene.planets[0]), 5000.0f, rand_interval(0.0f, 2 * 3.14f), 200);
	scene.planets[4].visual.shading.phong.specular = 0.3f;

	scene.planets[5] = Planet("M458", 0.07 / PhysicsComponent::G * 400, &(scene.planets[0]), 7000.0f, rand_interval(0.0f, 2 * 3.14f), 200);


	planet_index = 1;
}

void builFrustrsums(float midPlane) {
	float const aspect = SCR_WIDTH / static_cast<float>(SCR_HEIGHT);
	scene.farProjection = projection_perspective(pi / 3, aspect, midPlane, 10000.0f);
	scene.nearProjection = projection_perspective(pi / 3, aspect, 0.1f, midPlane);
	Planet::interPlane = midPlane;
}

void display_scene() {

	int nearPlanetIndex = -1;
	float midDistance = 100.0f;

	std::vector<Planet*> farPlanets;
	// Find the planet close to the player if it exists
	// Create an adaptative frustrum for the multipass render
	for (int i = 0; i < scene.planets.size(); i++) {
		float dist = vcl::norm(scene.planets[i].getPosition() - scene.camera.position());
		if (dist > midDistance + scene.planets[i].radius * 1.5f || nearPlanetIndex != -1)
			farPlanets.push_back(&scene.planets[i]);
		else {
			nearPlanetIndex = i;
			if (dist > midDistance - scene.planets[i].radius * 1.5f)
				builFrustrsums(midDistance + scene.planets[i].radius * 1.5f);
			else
				builFrustrsums(midDistance);
		}
	}
	if (nearPlanetIndex == -1)
		builFrustrsums(midDistance);


	// Render all the distant planets on the screen
	scene.projection = scene.farProjection;
	Planet::startPlanetRendering();
	for (int i = 0; i < farPlanets.size(); i++)
		if (i != 2)
			farPlanets[i]->renderPlanet(scene);

	scene.skybox.render(scene);

	Planet::startWaterRendering(scene, false);
	for (int i = farPlanets.size() - 1; i >= 1; i--) {
		Planet::switchIntermediateTexture();
		farPlanets[i]->renderWater(scene);
	}

	if (nearPlanetIndex == -1) {
		Planet::renderFinalPlanet();
		farPlanets[0]->renderWater(scene);
	}
	else {
		Planet::switchIntermediateTexture();
		farPlanets.back()->renderWater(scene);
		// We now need to render the near planet with the texture as the background.
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		scene.projection = scene.nearProjection;
		scene.planets[nearPlanetIndex].renderPlanet(scene);
		Planet::startWaterRendering(scene, true);
		Planet::renderFinalPlanet();
		scene.planets[nearPlanetIndex].renderWater(scene);
	}
}


void display_interface() {
	ImGui::Checkbox("Frame", &user.gui.display_frame);
	ImGui::Checkbox("Wireframe", &user.gui.display_wireframe);

	float camPos[3] = { scene.light.x, scene.light.y, scene.light.z };
	ImGui::SliderFloat3("Light position", camPos, 0.0f, 10.0f);
	scene.light = vec3(camPos[0], camPos[1], camPos[2]);

	ImGui::SliderInt("Planet index", &planet_index, 0, scene.planets.size() - 1);
	scene.planets[planet_index].displayInterface();
}


void window_size_callback(GLFWwindow* , int width, int height) {
	glViewport(0, 0, width, height);
	float const aspect = width / static_cast<float>(height);
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
	scene.farProjection = projection_perspective(pi/3, aspect, 100.0f, 10000.0f);
	scene.nearProjection = projection_perspective(pi/3, aspect, 0.1f, 100.0f);
	Planet::buildTextures(width, height);
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
}

void opengl_uniform(GLuint shader, scene_environment const& current_scene) {
	opengl_uniform(shader, "projection", current_scene.projection, false);
	opengl_uniform(shader, "view", scene.camera.matrix_view(), false);
	opengl_uniform(shader, "light", scene.light, false);
}


