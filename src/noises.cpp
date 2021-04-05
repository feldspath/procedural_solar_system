#include "noises.hpp"

bool displayPerlinNoiseGui(perlin_noise_parameters& parameters) {
	bool update = false;
	update |= ImGui::SliderFloat("Persistance", &parameters.persistency, 0.1f, 0.6f);
	update |= ImGui::SliderFloat("Frequency gain", &parameters.frequency_gain, 1.5f, 2.5f);
	update |= ImGui::SliderInt("Octave", &parameters.octave, 1, 8);
	update |= ImGui::SliderFloat3("Center", parameters.center, 0.0f, 1.0f);
	return update;
}

float ridgeNoise(vcl::vec3 const& p, perlin_noise_parameters& parameters, float sharpness) {
	vcl::vec3 sample = vcl::vec3(p.x + parameters.center[0] + 1.0f, p.y + parameters.center[1] + 1.0f, p.z + parameters.center[2] + 1.0f);
	float value = 0.0f;
	float a = 1.0f; // current magnitude
	float f = 1.0f; // current frequency
	for (int k = 0; k < parameters.octave; k++)
	{
		const float n = static_cast<float>(snoise3(sample.x * f, sample.y * f, sample.z * f));
		float v = 1 - std::abs(n);
		v = std::pow(v, sharpness);
		value += v * a;
		f *= parameters.frequency_gain;
		a *= parameters.persistency;
	}
	return value;
}

float perlinNoise(vcl::vec3 const& p, perlin_noise_parameters& parameters) {
	vcl::vec3 sample = vcl::vec3(p.x + parameters.center[0] + 1.0f, p.y + parameters.center[1] + 1.0f, p.z + parameters.center[2] + 1.0f);
	float value = 0.0f;
	float a = 1.0f; // current magnitude
	float f = 1.0f; // current frequency
	for (int k = 0; k < parameters.octave; k++)
	{
		const float n = static_cast<float>(snoise3(sample.x * f, sample.y * f, sample.z * f));
		value += a * n;
		f *= parameters.frequency_gain;
		a *= parameters.persistency;
	}
	return value;
}