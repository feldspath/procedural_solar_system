#pragma once

#include "vcl/vcl.hpp"


class Skybox {
private:
	vcl::mesh_drawable cube;

public:
	Skybox() {}
	Skybox(char* path);

	template <typename SCENE>
	void render(SCENE const& scene);
};

template <typename SCENE>
void Skybox::render(SCENE const& scene) {
	glUseProgram(cube.shader);
	vcl::mat4 posView = inverse(frame(scene.camera.orientation(), { 0, 0, 0 })).matrix();

	// Send uniforms for this shader
	vcl::opengl_uniform(cube.shader, "view", posView, false);
	vcl::opengl_uniform(cube.shader, "projection", scene.projection, false);

	// Set texture
	glActiveTexture(GL_TEXTURE0); opengl_check;
	glBindTexture(GL_TEXTURE_2D, cube.texture); opengl_check;
	vcl::opengl_uniform(cube.shader, "image_texture", 0, false);  opengl_check;

	// Call draw function
	glBindVertexArray(cube.vao);   opengl_check;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.vbo.at("index")); opengl_check;
	glDrawElements(GL_TRIANGLES, GLsizei(cube.number_triangles * 3), GL_UNSIGNED_INT, nullptr); opengl_check;

	// Clean buffers
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}