#pragma once

#include "vcl/vcl.hpp"

struct perlin_noise_parameters
{
	float persistency = 0.35f;
	float frequency_gain = 2.0f;
	int octave = 6;
	float influence = 0.5f;
};

void updatePlanetMesh(vcl::mesh& planet, float radius, perlin_noise_parameters const& parameters);
void updatePlanetVisual(vcl::mesh & planet, vcl::mesh_drawable & visual);