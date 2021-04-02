#pragma once

#include "vcl/vcl.hpp"

struct perlin_noise_parameters
{
	float persistency = 0.35f;
	float frequency_gain = 2.0f;
	int octave = 6;
	float influence = 0.5f;

	float center[3];
};

class Planet {

private:

	vcl::mesh m;
	vcl::mesh_drawable visual;

	GLuint shader;

public:

	float radius = 1.0f;
	float waterLevel = 1.5f;
	float depthMultiplier = 6.0f;
	float waterBlendMultiplier = 60.0f;


	Planet() {}

	Planet(float r, float level, GLuint shader = vcl::mesh_drawable::default_shader);

	void updatePlanetMesh(perlin_noise_parameters &parameters);
	vcl::mesh_drawable& getVisual();

	float& getRadiusRef() {
		return radius;
	}

	float& getWaterLevelRef() {
		return waterLevel;
	}

};

