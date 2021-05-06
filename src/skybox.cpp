#include "skybox.hpp"
#include "vcl/vcl.hpp"


Skybox::Skybox(char* path) {
	// Skybox
	vcl::mesh cubemap = vcl::mesh_primitive_cube();
	int count = 0;
	float offset = 0.001f;
	// Front
	cubemap.uv[count++] = { 0.25f, 1.0f - (1.0f + offset) / 3 };
	cubemap.uv[count++] = { 0.5f , 1.0f - (1.0f + offset) / 3 };
	cubemap.uv[count++] = { 0.5f , 1.0f - (2.0f - offset) / 3 };
	cubemap.uv[count++] = { 0.25f, 1.0f - (2.0f - offset) / 3 };
	// Right
	cubemap.uv[count++] = { 0.5f , 1.0f - (1.0f + offset) / 3 };
	cubemap.uv[count++] = { 0.75f, 1.0f - (1.0f + offset) / 3 };
	cubemap.uv[count++] = { 0.75f, 1.0f - (2.0f - offset) / 3 };
	cubemap.uv[count++] = { 0.5f , 1.0f - (2.0f - offset) / 3 };
	// Back
	cubemap.uv[count++] = { 0.75f, 1.0f - (1.0f + offset) / 3 };
	cubemap.uv[count++] = { 1.0f , 1.0f - (1.0f + offset) / 3 };
	cubemap.uv[count++] = { 1.0f , 1.0f - (2.0f - offset) / 3 };
	cubemap.uv[count++] = { 0.75f, 1.0f - (2.0f - offset) / 3 };
	// Left
	cubemap.uv[count++] = { 0.0f , 1.0f - (1.0f + offset) / 3 };
	cubemap.uv[count++] = { 0.25f, 1.0f - (1.0f + offset) / 3 };
	cubemap.uv[count++] = { 0.25f, 1.0f - (2.0f - offset) / 3 };
	cubemap.uv[count++] = { 0.0f , 1.0f - (2.0f - offset) / 3 };
	// Top
	cubemap.uv[count++] = { 0.25f + offset, 1.0f - 2.0f / 3 };
	cubemap.uv[count++] = { 0.5f - offset, 1.0f - 2.0f / 3 };
	cubemap.uv[count++] = { 0.5f - offset, 1.0f - 1.0f };
	cubemap.uv[count++] = { 0.25f + offset, 1.0f - 1.0f };
	// Bottom
	cubemap.uv[count++] = { 0.5f - offset, 1.0f - 1.0f / 3 };
	cubemap.uv[count++] = { 0.25f + offset, 1.0f - 1.0f / 3 };
	cubemap.uv[count++] = { 0.25f + offset, 1.0f - 0.0f };
	cubemap.uv[count++] = { 0.5f - offset, 1.0f - 0.0f };

	cube = vcl::mesh_drawable(cubemap);
	cube.texture = vcl::opengl_texture_to_gpu(vcl::image_load_png(path));
	cube.shader = vcl::opengl_create_shader_program(vcl::read_text_file("shaders/skybox/skybox.vert.glsl"), vcl::read_text_file("shaders/skybox/skybox.frag.glsl"));
}