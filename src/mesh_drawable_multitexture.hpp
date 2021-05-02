#pragma once

#include "vcl/vcl.hpp"

// Create a mesh_drawable_multitexture in reusing the mesh_drawable structure and adding an extra texture ID
struct mesh_drawable_multitexture : vcl::mesh_drawable {

	using mesh_drawable::mesh_drawable; // reuse the same constructor

	GLuint texture_2; // add a second texture
};


// Adapt the draw function for a mesh_drawable_multitexture
template <typename SCENE>
void draw(mesh_drawable_multitexture const& drawable, SCENE const& scene)
{
	// Reuse the same draw function, but send an additional uniform for the second texture
	// ...

	assert_vcl(drawable.shader != 0, "Try to draw mesh_drawable without shader");
	assert_vcl(drawable.texture != 0, "Try to draw mesh_drawable without texture");
	glUseProgram(drawable.shader); opengl_check;
	opengl_uniform(drawable.shader, scene);
	opengl_uniform(drawable.shader, drawable.shading, false);
	opengl_uniform(drawable.shader, "model", drawable.transform.matrix(), false);

	// Set the two textures
	glActiveTexture(GL_TEXTURE0); opengl_check;
	glBindTexture(GL_TEXTURE_2D, drawable.texture); opengl_check;
	vcl::opengl_uniform(drawable.shader, "image_texture", 0);  opengl_check;

	glActiveTexture(GL_TEXTURE1); opengl_check; // the additional texture 
	glBindTexture(GL_TEXTURE_2D, drawable.texture_2); opengl_check;
	vcl::opengl_uniform(drawable.shader, "image_texture_2", 1);  opengl_check; // the second texture is called "image_texture_2" in the shader in this case

	// Standard call function
	assert_vcl(drawable.number_triangles > 0, "Try to draw mesh_drawable with 0 triangles"); opengl_check;
	glBindVertexArray(drawable.vao);   opengl_check;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawable.vbo.at("index")); opengl_check;
	glDrawElements(GL_TRIANGLES, GLsizei(drawable.number_triangles * 3), GL_UNSIGNED_INT, nullptr); opengl_check;


	glBindVertexArray(0);

	// Clean the two textures binding to avoid any side effect after this draw
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}