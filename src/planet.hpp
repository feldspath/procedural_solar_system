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
	float radius;

	GLuint shader;
	float waterLevel;

public:

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

