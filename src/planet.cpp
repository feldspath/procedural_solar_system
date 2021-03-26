#include "planet.hpp"

using namespace vcl;

static vec3 getPlanetRadiusAt(vec3& position, float radius, perlin_noise_parameters const& parameters) {
    const vec3 posOnUnitSphere = position.normalize();
    float const noise = noise_perlin(posOnUnitSphere, parameters.octave, parameters.persistency, parameters.frequency_gain);
    float const elevation = (noise + 1.0f) * parameters.influence;
    vec3 newPosition = radius * posOnUnitSphere * (1 + elevation);
    return newPosition;
}

void updatePlanetMesh(mesh& planet, float radius, perlin_noise_parameters const& parameters) {
    for (int i = 0; i < planet.position.size(); i++) {
        planet.position[i] = getPlanetRadiusAt(planet.position[i], radius, parameters);
    }
    planet.compute_normal();
}

void updatePlanetVisual(mesh& planet, mesh_drawable& visual) {
    visual.update_position(planet.position);
    visual.update_normal(planet.normal);
    visual.update_color(planet.color);
}
