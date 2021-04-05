#pragma once

#include "third_party/src/simplexnoise/simplexnoise1234.hpp"
#include "vcl/vcl.hpp"

struct perlin_noise_parameters
{
	float persistency = 0.35f;
	float frequency_gain = 2.0f;
	int octave = 6;

	float center[3] = { 0.0f, 0.0f, 0.0f };
};

bool displayPerlinNoiseGui(perlin_noise_parameters& parameters);

float ridgeNoise(vcl::vec3 const& p, perlin_noise_parameters& parameters, float sharpness);

float perlinNoise(vcl::vec3 const& p, perlin_noise_parameters& parameters);