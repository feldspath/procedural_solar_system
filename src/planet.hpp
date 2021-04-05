#pragma once

#include "vcl/vcl.hpp"
#include "noises.hpp"

class Planet {

private:

	vcl::mesh m;
	GLuint shader = 0;

public:
	vcl::mesh_drawable visual;

	float radius = 1.0f;
	perlin_noise_parameters perlinParameters;
	perlin_noise_parameters mountainsParameters;
	perlin_noise_parameters maskParameters;
	float mountainSharpness = 1.0f;
	float textureScale = 1.0f;
	float textureSharpness = 5.0f;
	float normalMapInfluence = 0.2f;

	float oceanFloorDepth = 0.5f;
	float oceanFloorSmoothing = 1.0f;
	float oceanDepthMultiplier = 2.0f;
	float mountainsBlend = 1.0f;
	float maskShift = 0.0f;

	float rotateSpeed = 0.0f;

	Planet() {}

	Planet(float r, GLuint shader = vcl::mesh_drawable::default_shader);

	void updatePlanetMesh();

	void setCustomUniforms();
	void setTexture(GLuint texture);

	void rotatePlanet(float deltaTime) {
		vcl::rotation rot({ 0.0f, 0.0f, 1.0f }, deltaTime * rotateSpeed);
		visual.transform.rotate = rot * visual.transform.rotate;
	}

	vcl::vec3 getPlanetRadiusAt(vcl::vec3& position);

};

