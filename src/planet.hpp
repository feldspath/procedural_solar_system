#pragma once

#include "vcl/vcl.hpp"
#include "noises.hpp"
#include "mesh_drawable_multitexture.hpp"

class Planet {

private:

	vcl::mesh m;
	GLuint shader = 0;

public:

	vcl::mesh_drawable visual;
	mesh_drawable_multitexture postProcessingQuad;
	GLuint fbo;                // Frame buffer for multi-pass render
	GLuint depth_buffer;       // Depth buffer used when rendered in the frame buffer
	GLuint intermediate_image; // Texture of the rendered color image

	float radius = 1.0f;
	float rotateSpeed = 0.0f;

	// Continent
	perlin_noise_parameters continentParameters;

	// Mountains
	perlin_noise_parameters mountainsParameters;
	perlin_noise_parameters maskParameters;

	float mountainSharpness = 1.0f;
	float mountainsBlend = 1.0f;
	float maskShift = 0.0f;

	// Oceans
	float oceanFloorDepth = 0.5f;
	float oceanFloorSmoothing = 1.0f;
	float oceanDepthMultiplier = 2.0f;

	// Texture
	float textureScale = 1.0f;
	float textureSharpness = 5.0f;
	float normalMapInfluence = 0.2f;

	Planet() {}

	Planet(float r, const unsigned int width, const unsigned int height, int division = 200);

	template <typename SCENE>
	void render(SCENE const& scene) {
		glEnable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		setCustomUniforms();
		draw(visual, scene);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);

		glUseProgram(postProcessingQuad.shader);

		// Matrices
		opengl_uniform(postProcessingQuad.shader, "viewMatrix", scene.camera.matrix_view(), true);
		opengl_uniform(postProcessingQuad.shader, "perspectiveInverse", inverse(scene.projection), true);

		// Terrain
		vcl::vec4 center = vec4(planet.visual.transform.translate, 1.0f);
		opengl_uniform(postProcessingQuad.shader, "planetCenter", center);
		draw(postProcessingQuad, scene);
	}

	void updatePlanetMesh();

	void setCustomUniforms();
	void setTexture(GLuint texture);

	void rotatePlanet(float deltaTime);

	vcl::vec3 getPlanetRadiusAt(vcl::vec3& position);
	void buildTextures(const unsigned int width, const unsigned int height);

private:
	void buildFbo(const unsigned int width, const unsigned int height);
	

};

