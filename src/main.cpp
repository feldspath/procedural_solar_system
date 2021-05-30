#include "vcl/vcl.hpp"
#include <iostream>
#include <time.h>
#include <vector>
#include <thread>
#include <algorithm>

#include "planet.hpp"
#include "physics.hpp"
#include "skybox.hpp"
#include "camera_fps.hpp"
#include "player.hpp"
#include "vegetation.hpp"

// 0 is edit mode
// 1 is explore mode	
#define CAMERA_TYPE 0

using namespace vcl;

struct gui_parameters {
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
	gui_parameters gui;
	bool cursor_on_gui;
	keyboard_state_parameters keyboard_state;
};
user_interaction_parameters user;


struct scene_environment
{
#if CAMERA_TYPE
	camera_fps camera;
#else
	camera_around_center camera;
#endif
	mat4 projection;
	mat4 nearProjection;
	mat4 farProjection;
	vec3 light;
	Player player;
	std::vector<Planet> planets;
	Skybox skybox;

	hierarchy_mesh_drawable plant;
	buffer<buffer<float>> plantInfos;
};

scene_environment scene;


void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
void window_size_callback(GLFWwindow* window, int width, int height);

void initialize_data();
void display_scene();
void display_interface();

unsigned int SCR_WIDTH = 1280;
unsigned int SCR_HEIGHT = 1024;

#if CAMERA_TYPE
float midDistance = 30.0f;
#else
float midDistance = 5.0f;
#endif

int resolution = 500;

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

#if !CAMERA_TYPE
		ImGui::Begin("GUI",NULL,ImGuiWindowFlags_AlwaysAutoResize);
		user.cursor_on_gui = ImGui::IsAnyWindowFocused();	
#endif

		display_scene();

		// Clean buffers
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		

#if !CAMERA_TYPE
		display_interface();
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
	GLuint const shader_mesh = opengl_create_shader_program(opengl_shader_preset("mesh_vertex"), opengl_shader_preset("mesh_fragment"));
	GLuint const shader_uniform_color = opengl_create_shader_program(opengl_shader_preset("single_color_vertex"), opengl_shader_preset("single_color_fragment"));
	GLuint const texture_white = opengl_texture_to_gpu(image_raw{1,1,image_color_type::rgba,{255,255,255,255}});
	mesh_drawable::default_shader = shader_mesh;
	mesh_drawable::default_texture = texture_white;
	curve_drawable::default_shader = shader_uniform_color;
	segments_drawable::default_shader = shader_uniform_color;	

	float sun_mass = (float)1e16;

	#if CAMERA_TYPE
		scene.camera.position_camera = { 0, 0, 0};
		scene.camera.orientation_camera = rotation();
		scene.player.bind_camera(&scene.camera);
		scene.player.bind_planets(&scene.planets);
		PhysicsComponent::player = &scene.player;
	#else
	#endif

	scene.skybox = Skybox("assets/cubemap.png");

    Planet::initPlanetRenderer(SCR_WIDTH, SCR_HEIGHT);
	int nPlanets = 9;
	scene.planets.resize(nPlanets);
	// Sun
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

	planet_index = 2;

	createPlant(scene.plant);
	scene.plantInfos = plantSpawn(5000, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), 0.5);
}

void drawPlant(vec3 position, float alpha, float scale) {
	scene.plant["troncon 0"].transform.translate = scene.planets[2].getPlanetRadiusAt(position) * 0.999f;
	if (norm(scene.plant["troncon 0"].transform.translate + scene.planets[2].getPosition()) > 15.0f) {
		return;
	}
	vec3 up = position;
	vec3 ortho = vec3(1.0f, 0.0f, 0.0f);
	vec3 tempCross = cross(up, ortho);
	if (norm(tempCross) < 0.01f)
		return;
	vec3 right = normalize(tempCross);
	vec3 back = cross(right, up);
	mat3 rotationMatrix({back, right, up });
	scene.plant["troncon 0"].transform.rotate = rotation(up, alpha) * rotation(transpose(rotationMatrix));
	scene.plant["troncon 0"].transform.scale = scale;
	scene.plant["troncon 0"].transform = scene.planets[2].visual.transform * scene.plant["troncon 0"].transform;
	scene.plant.update_local_to_global_coordinates();
	draw(scene.plant, scene);
}

void buildFrustrsums(float interPlane) {
	float farPlane = 10000.0f;
	float nearPlane = 0.1f;
	float const aspect = SCR_WIDTH / static_cast<float>(SCR_HEIGHT);
	scene.farProjection = projection_perspective(pi / 3, aspect, interPlane, farPlane);
	scene.nearProjection = projection_perspective(pi / 3, aspect, nearPlane, interPlane);
	Planet::interPlane = interPlane;
	Planet::nearPlane = nearPlane;
	Planet::farPlane = farPlane;
}

struct SortingPlanet {
	Planet* pointer;
	float distance;

	SortingPlanet(Planet* planet, float distance) {
		pointer = planet;
		this->distance = distance;
	}
};

bool operator<(const SortingPlanet& first, const SortingPlanet& second)
{
	return first.distance < second.distance;
}


void display_scene() {
	plantAnimation(scene.plant, user.fps_record.t * 0.5f);

	scene.light = scene.planets[0].getPosition();
	
	// Find the planet close to the player if it exists
	// Create an adaptative frustrum for the multipass render
	std::vector<SortingPlanet> farPlanets;
	std::vector<int> nearPlanetIndices;
	float separatingPlane = midDistance;
	for (int i = 0; i < scene.planets.size(); i++) {
		//float dist = vcl::norm(scene.planets[i].getPosition() - scene.camera.position());
		float dist = vcl::norm(scene.planets[i].getPosition() - scene.camera.position());
		float changeDist = scene.planets[i].radius * (scene.planets[i].hasAtmosphere ? std::max(scene.planets[i].atmosphereHeight, 1.3f) : 1.3f);
		if (dist > separatingPlane + changeDist || !CAMERA_TYPE) {
			if (dot(scene.camera.front(), scene.planets[i].getPosition()) > 0 || !CAMERA_TYPE)
				farPlanets.push_back(SortingPlanet(&scene.planets[i], dist));
		}
		else {
			nearPlanetIndices.push_back(i);
			if (dist > separatingPlane - changeDist)
				separatingPlane += changeDist;				
		}
	}

	if (nearPlanetIndices.empty())
		buildFrustrsums(midDistance);
	else {
		buildFrustrsums(separatingPlane);
	}
	

	std::sort(farPlanets.begin(), farPlanets.end());

	// Render all the distant planets on the screen
	scene.projection = scene.farProjection;
	Planet::startPlanetRendering();
	for (int i = 0; i < farPlanets.size(); i++) {
		farPlanets[i].pointer->renderPlanet(scene, farPlanets[i].distance > 200.0f);
	}

	if (farPlanets.size() > 0 || farPlanets.empty())
		scene.skybox.render(scene);

	Planet::startWaterRendering(scene, false);
	for (int i = farPlanets.size() - 1; i >= 1; i--) {
		Planet::switchIntermediateTexture();
		farPlanets[i].pointer->renderWater(scene);
	}

	if (nearPlanetIndices.empty()) {
		Planet::renderFinalPlanet();
		if (farPlanets.size() > 0)
			farPlanets[0].pointer->renderWater(scene);
		else
			scene.skybox.render(scene);
	}
	else {
		if (farPlanets.size() > 0) {
			Planet::switchIntermediateTexture();
			farPlanets[0].pointer->renderWater(scene);
		}
		// We now need to render the near planet with the texture as the background.
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		scene.projection = scene.nearProjection;
		for (int i = 0; i < nearPlanetIndices.size(); i++) {
			scene.planets[nearPlanetIndices[i]].renderPlanet(scene);
			if (nearPlanetIndices[i] == 2) {
				for (int j = 0; j < scene.plantInfos[0].size(); j++) {
					vec3 pos = vcl::vec3(scene.plantInfos[2][j], scene.plantInfos[3][j], scene.plantInfos[4][j]);
					drawPlant(pos, scene.plantInfos[5][j], scene.plantInfos[9][j] / 4);
				}
			}
		}
		
		Planet::startWaterRendering(scene, true);
		for (int i = 0; i < nearPlanetIndices.size()-1; i++) {
			Planet::switchIntermediateTexture();
			scene.planets[nearPlanetIndices[i]].renderWater(scene);
		}
		Planet::renderFinalPlanet();
		scene.planets[nearPlanetIndices.back()].renderWater(scene);
	}
}


void display_interface() {
	float camPos[3] = { scene.light.x, scene.light.y, scene.light.z };
	ImGui::SliderFloat3("Light position", camPos, 0.0f, 10.0f);
	scene.light = vec3(camPos[0], camPos[1], camPos[2]);

	ImGui::SliderInt("Planet index", &planet_index, 0, scene.planets.size() - 1);
	scene.planets[planet_index].displayInterface();
}


void window_size_callback(GLFWwindow* , int width, int height) {
	glViewport(0, 0, width, height);
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
	buildFrustrsums(midDistance);
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
	if (key == GLFW_KEY_E && action == GLFW_PRESS) {
		scene.player.toggleJetpack();
	}
}

void opengl_uniform(GLuint shader, scene_environment const& current_scene) {
	opengl_uniform(shader, "projection", current_scene.projection, false);
	opengl_uniform(shader, "view", scene.camera.matrix_view(), false);
	opengl_uniform(shader, "light", scene.light, false);
}


