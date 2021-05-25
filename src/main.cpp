#include "vcl/vcl.hpp"
#include <iostream>
#include <time.h>

#include "planet.hpp"
#include "physics.hpp"
#include "skybox.hpp"
#include "camera_fps.hpp"

#define N_PLANETS 4

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
	vec3 light;
};
#else
struct scene_environment
{
	camera_around_center camera;
	mat4 projection;
	vec3 light;
};
#endif

scene_environment scene;

void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
void window_size_callback(GLFWwindow* window, int width, int height);

void initialize_data();
void display_scene();
void display_interface();

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 1024;

float previousTime;
float deltaTime;

Planet planets[N_PLANETS];
int planet_index;

Skybox skybox;

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
		deltaTime = glfwGetTime() - previousTime;
		previousTime = glfwGetTime();


		// Camera 
		#if CAMERA_TYPE
			vec3 direction = { user.keyboard_state.front, user.keyboard_state.right, user.keyboard_state.up };
			scene.camera.update_position(direction);
		#else
			scene.camera.center_of_rotation = planets[planet_index].getPosition();
			scene.light = scene.camera.position();
		#endif
		
		// Physics
		PhysicsComponent::update(deltaTime);
		for (int i = 0; i < N_PLANETS; i++)
			planets[i].updateRotation(deltaTime);
		
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

	float sun_mass = (float)1e12;

	#if CAMERA_TYPE
		scene.camera.position_camera = { 25, 5, 10};
		scene.camera.orientation_camera = rotation();
		scene.camera.init_physics();
	#else
	#endif

    Planet::initPlanetRenderer(SCR_WIDTH, SCR_HEIGHT);

	

	planets[0] = Planet(2.0f, sun_mass, { 0, 0, 0 }, {0, 0, 0}, 100, true);
	planets[1] = Planet(1.0f, 1e7, { 10, 0, 0 }, {0, sqrt(PhysicsComponent::G * sun_mass / 10) + 0.3f, 0.1f}, 100, true);
	planets[2] = Planet(1.0f, 3e10, { 25, 0, 0 }, {0, sqrt(PhysicsComponent::G * sun_mass / 25) + 0.1f, 0.01f}, 100, true);
	planets[3] = Planet(0.1f, 1e5, { 28, 0, 0 }, {-0.4f, sqrt(PhysicsComponent::G * sun_mass / 25) + 0.8f, 0.0f}, 100, true);

	planet_index = 0;

	// Light
	scene.light = { 0.0f, 0.0f, 0.0f };

	skybox = Skybox("assets/cubemap.png");

	planets[0].importFromFile("planets/test6.pbf");
	//planets[1].importFromFile("planets/rocky.txt");
	//planets[2].importFromFile("planets/livable.txt");
	//planets[3].importFromFile("planets/sat1.txt");
}


void display_scene()
{
    Planet::startPlanetRendering();
	for (int i = 0; i < N_PLANETS; i++)
		planets[i].renderPlanet(scene);

	skybox.render(scene);
   
    Planet::startWaterRendering(scene);
	for (int i = 0; i < N_PLANETS - 1; i++) {
		Planet::switchIntermediateTexture();
		planets[i].renderWater(scene);
	}

	Planet::renderFinalPlanet();
	planets[N_PLANETS-1].renderWater(scene);
    

}


void display_interface()
{
	ImGui::Checkbox("Frame", &user.gui.display_frame);
	ImGui::Checkbox("Wireframe", &user.gui.display_wireframe);

	float camPos[3] = { scene.light.x, scene.light.y, scene.light.z };
	ImGui::SliderFloat3("Light position", camPos, 0.0f, 10.0f);
	scene.light = vec3(camPos[0], camPos[1], camPos[2]);

	ImGui::SliderInt("Planet index", &planet_index, 0, N_PLANETS - 1);
	planets[planet_index].displayInterface();
}


void window_size_callback(GLFWwindow* , int width, int height)
{
	glViewport(0, 0, width, height);
	float const aspect = width / static_cast<float>(height);
	scene.projection = projection_perspective(pi/3, aspect, 0.1f, 100.0f);
	Planet::buildTextures(width, height);
}
	

void mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
{
	vec2 const  p1 = glfw_get_mouse_cursor(window, xpos, ypos);
	vec2 const& p0 = user.mouse_prev;
	glfw_state state = glfw_current_state(window);

	auto& camera = scene.camera;
	if(!user.cursor_on_gui){
		#if CAMERA_TYPE
			vec2 offset = p1 - p0;
			offset.x = -offset.x;
			scene.camera.update_orientation(offset);
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

void opengl_uniform(GLuint shader, scene_environment const& current_scene)
{
	opengl_uniform(shader, "projection", current_scene.projection, false);
	opengl_uniform(shader, "view", scene.camera.matrix_view(), false);
	opengl_uniform(shader, "light", scene.light, false);
}


