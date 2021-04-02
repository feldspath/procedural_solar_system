#include "planet.hpp"
#include "icosphere.hpp"

using namespace vcl;

static vec3 getPlanetRadiusAt(vec3& position, float radius, perlin_noise_parameters const& parameters) {
    const vec3 posOnUnitSphere = normalize(position);
    const vec3 sample = { posOnUnitSphere.x + parameters.center[0] + 2.0f, posOnUnitSphere.y + parameters.center[1] + 2.0f, posOnUnitSphere.z + parameters.center[2] + 2.0f };
    float const noise = noise_perlin(sample, parameters.octave, parameters.persistency, parameters.frequency_gain);
    float const elevation = (noise + 1.0f) * 0.5f;
    vec3 newPosition = radius * posOnUnitSphere * (1 + elevation * parameters.influence);
    return newPosition;
}

Planet::Planet(float r, float level, GLuint shader) {
    radius = r;
    waterLevel = level;
    //m = mesh_primitive_sphere();
    m = createIcoSphere(radius, 200);
    visual = mesh_drawable(m, shader);
    visual.shading.color = { 0.4, 0.35, 0.25 };
    visual.shading.phong.specular = 0.0f;
}

void Planet::updatePlanetMesh(perlin_noise_parameters &parameters) {
    for (int i = 0; i < m.position.size(); i++) {
        m.position[i] = getPlanetRadiusAt(m.position[i], radius, parameters);
    }
    m.compute_normal();

    visual.update_position(m.position);
    visual.update_normal(m.normal);
    visual.update_color(m.color);
}

mesh_drawable& Planet::getVisual() {
    return visual;
}
