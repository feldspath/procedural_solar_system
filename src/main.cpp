#include "vcl/vcl.hpp"
#include <iostream>
#include <time.h>

#include "planet.hpp"
#include "post_processing.hpp"
#include "noises.hpp"


using namespace vcl;

struct gui_parameters {
	bool display_frame = false;
	bool display_wireframe = false;
};

struct user_interaction_parameters {
	vec2 mouse_prev;
	timer_fps fps_record;
	mesh_drawable global_frame;
	gui_parameters gui;
	bool cursor_on_gui;
};
user_interaction_parameters user;

struct scene_environment
{
	camera_around_center camera;
	mat4 projection;
	vec3 light;
};
scene_environment scene;



void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
void window_size_callback(GLFWwindow* window, int width, int height);

std::string openShader(std::string const& shader_name);

void initialize_data();
void display_scene();
void display_interface();

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 1024;

Planet planet;
PostProcessing waterPostProc;

float previousTime;
float deltaTime;


int main(int, char* argv[])
{
	std::cout << "Run " << argv[0] << std::endl;

	int const width = SCR_WIDTH;
	int const height = SCR_HEIGHT;
	GLFWwindow* window = create_window(width, height);
	window_size_callback(window, width, height);
	std::cout << opengl_info_display() << std::endl;;

	imgui_init(window);
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
		planet.rotatePlanet(deltaTime);
		scene.light = scene.camera.position();
		user.fps_record.update();

		waterPostProc.startRenderToFrameBuffer();
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		display_scene();
		waterPostProc.stopRenderToFrameBuffer();

		imgui_create_frame();
		if(user.fps_record.event) {
			std::string const title = "VCL Display - "+str(user.fps_record.fps)+" fps";
			glfwSetWindowTitle(window, title.c_str());
		}

		ImGui::Begin("GUI",NULL,ImGuiWindowFlags_AlwaysAutoResize);
		user.cursor_on_gui = ImGui::IsAnyWindowFocused();
		
		if(user.gui.display_frame) draw(user.global_frame, scene);

		waterPostProc.renderColorbuffer(scene.camera, scene.projection, planet);
		//display_scene();
		
		display_interface();

		ImGui::End();
		imgui_render_frame(window);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	vcl::imgui_cleanup();
	glfwDestroyWindow(window);
	glfwTerminate();

	PostProcessing::deleteRenderQuad();

	return 0;
}


void initialize_data()
{
	GLuint const shader_mesh = opengl_create_shader_program(opengl_shader_preset("mesh_vertex"), opengl_shader_preset("mesh_fragment"));
	GLuint const shader_uniform_color = opengl_create_shader_program(opengl_shader_preset("single_color_vertex"), opengl_shader_preset("single_color_fragment"));
	GLuint const shader_post_processing = opengl_create_shader_program(opengl_shader_preset("post_processing_vertex"), opengl_shader_preset("post_processing_fragment"));
	GLuint const texture_white = opengl_texture_to_gpu(image_raw{1,1,image_color_type::rgba,{255,255,255,255}});
	mesh_drawable::default_shader = shader_mesh;
	mesh_drawable::default_texture = texture_white;
	curve_drawable::default_shader = shader_uniform_color;
	segments_drawable::default_shader = shader_uniform_color;	
	PostProcessing::defaultShader = shader_post_processing;
	PostProcessing::initialiseRenderQuad();

	GLuint postProcShader = opengl_create_shader_program(openShader("planet_post_vertex"), openShader("planet_post_fragment"));
	waterPostProc = PostProcessing(postProcShader, SCR_WIDTH, SCR_HEIGHT);
	
	user.global_frame = mesh_drawable(mesh_primitive_frame());
	user.gui.display_frame = false;

	scene.camera.distance_to_center = 2.5f;
	scene.camera.look_at({4,3,2}, {0,0,0}, {0,0,1});


	// Planet init
	GLuint planetShader = opengl_create_shader_program(openShader("planet_vertex"), openShader("planet_fragment"));

	//image_raw const im = image_load_png("assets/checker_texture.png");
	image_raw const im = image_load_png("assets/moon_normal_map1.png");
	GLuint const planetTexture = opengl_texture_to_gpu(im, GL_REPEAT, GL_REPEAT);
	planet = Planet(1.0f, planetShader);
	planet.setTexture(planetTexture);
	planet.updatePlanetMesh();

	// Light
	scene.light = { 2.0f, 3.0f, 2.0f };
	
}


void display_scene()
{
	mesh_drawable &planet_visual = planet.visual;
	planet.setCustomUniforms();
	draw(planet_visual, scene);
	if (user.gui.display_wireframe) {
		draw_wireframe(planet_visual, scene, { 1, 0, 0 });
	}
}


void display_interface()
{
	ImGui::Checkbox("Frame", &user.gui.display_frame);
	ImGui::Checkbox("Wireframe", &user.gui.display_wireframe);

	float camPos[3] = { scene.light.x, scene.light.y, scene.light.z };
	ImGui::SliderFloat3("Light position", camPos, 0.0f, 10.0f);
	scene.light = vec3(camPos[0], camPos[1], camPos[2]);

	bool update = false;
	if (ImGui::CollapsingHeader("Planet parameters")) {

		ImGui::SliderFloat("Rotation speed", &planet.rotateSpeed, 0.0f, 3.0f);

		float col[3] = { planet.visual.shading.color.x, planet.visual.shading.color.y , planet.visual.shading.color.z };
		ImGui::ColorEdit3("Planet color", col);
		planet.visual.shading.color = vec3(col[0], col[1], col[2]);


		if (ImGui::TreeNode("Terrain generation")) {
			update |= ImGui::SliderFloat("Radius", &planet.radius, 0.0f, 3.0f);
			if (ImGui::TreeNode("Continent noise")) {
				update |= displayPerlinNoiseGui(planet.perlinParameters);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Mountain noise")) {
				update |= ImGui::SliderFloat("Sharpness", &planet.mountainSharpness, 0.1f, 5.0f);
				update |= displayPerlinNoiseGui(planet.mountainsParameters);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Oceans")) {
				update |= ImGui::SliderFloat("Floor depth", &planet.oceanFloorDepth, 0.0f, 1.0f);
				update |= ImGui::SliderFloat("Floor smoothing", &planet.oceanFloorSmoothing, 0.1f, 20.0f);
				update |= ImGui::SliderFloat("Depth multiplier", &planet.oceanDepthMultiplier, 0.0f, 10.0f);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Moutains Mask")) {
				update |= ImGui::SliderFloat("Moutains blend", &planet.mountainsBlend, 0.1f, 20.0f);
				update |= ImGui::SliderFloat("Vertical shift", &planet.maskShift, -2.0f, 2.0f);
				update |= displayPerlinNoiseGui(planet.maskParameters);
				ImGui::TreePop();
			}

			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Water")) {
			ImGui::SliderFloat("Water level", &waterPostProc.waterLevel, 0.0f, 3.0f);

			// Water Color
			float colD[3] = {waterPostProc.waterColorSurface.x, waterPostProc.waterColorSurface.y , waterPostProc.waterColorSurface.z};
			ImGui::ColorEdit3("Surface water", colD);
			waterPostProc.waterColorSurface = vec3(colD[0], colD[1], colD[2]);
			float colS[3] = { waterPostProc.waterColorDeep.x, waterPostProc.waterColorDeep.y , waterPostProc.waterColorDeep.z };
			ImGui::ColorEdit3("Deep water", colS);
			waterPostProc.waterColorDeep = vec3(colS[0], colS[1], colS[2]);

			ImGui::SliderFloat("Depth multiplier", &waterPostProc.depthMultiplier, 0.0f, 10.0f);
			ImGui::SliderFloat("Water blend multipler", &waterPostProc.waterBlendMultiplier, 0.0f, 100.0f);
			ImGui::TreePop();

		}
		if (ImGui::TreeNode("Texture")) {
			ImGui::SliderFloat("Scale", &planet.textureScale, 0.1f, 2.0f);
			ImGui::SliderFloat("Sharpness", &planet.textureSharpness, 0.1f, 5.0f);
			ImGui::SliderFloat("Normal map influence", &planet.normalMapInfluence, 0.0f, 1.0f);
			ImGui::TreePop();
		}
	}

	if (update)
		planet.updatePlanetMesh();
}


void window_size_callback(GLFWwindow* , int width, int height)
{
	glViewport(0, 0, width, height);
	float const aspect = width / static_cast<float>(height);
	scene.projection = projection_perspective(pi/3, aspect, 0.1f, 100.0f);
	waterPostProc.buildTextures(width, height);
}
	

void mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
{
	vec2 const  p1 = glfw_get_mouse_cursor(window, xpos, ypos);
	vec2 const& p0 = user.mouse_prev;
	glfw_state state = glfw_current_state(window);

	auto& camera = scene.camera;
	if(!user.cursor_on_gui){
		if(state.mouse_click_left && !state.key_ctrl)
			camera.manipulator_rotate_trackball(p0, p1);
		if(state.mouse_click_left && state.key_ctrl)
			camera.manipulator_translate_in_plane(p1-p0);
		if(state.mouse_click_right)
			camera.manipulator_scale_distance_to_center( (p1-p0).y );
	}

	user.mouse_prev = p1;
}

void opengl_uniform(GLuint shader, scene_environment const& current_scene)
{
	opengl_uniform(shader, "projection", current_scene.projection);
	opengl_uniform(shader, "view", scene.camera.matrix_view());
	opengl_uniform(shader, "light", scene.light, false);
}

std::string openShader(std::string const& shader_name)
{
	if (shader_name == "planet_post_vertex") {
		#include "../shaders/planet_post/planet.vert.glsl"
		return s;
	}
	if (shader_name == "planet_post_fragment") {
		#include "../shaders/planet_post/planet.frag.glsl"
		return s;
	}
	if (shader_name == "planet_vertex") {
		#include "../shaders/planet/planet.vert.glsl"
		return s;
	}
	if (shader_name == "planet_fragment") {
		#include "../shaders/planet/planet.frag.glsl"
		return s;
	}

	error_vcl("Shader not found");
	return "Error";
}


