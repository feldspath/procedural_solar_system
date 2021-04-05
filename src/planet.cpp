#include "planet.hpp"
#include "icosphere.hpp"
#include "vcl/vcl.hpp"
#include "noises.hpp"

using namespace vcl;

static float smoothMax(float a, float b, float k) {
    return std::log(std::exp(a * k) + std::exp(b * k)) / k;
}

static float blend(float noise, float blending) {
    return std::atan(noise * blending) / pi + 0.5f;
}

vec3 Planet::getPlanetRadiusAt(vec3& position) {
    const vec3 posOnUnitSphere = normalize(position);

    // Continent (simple perlin noise)
    float const perlin_noise = perlinNoise(posOnUnitSphere, perlinParameters);
    
    // Moutains
    float moutainMask = blend(perlinNoise(posOnUnitSphere, maskParameters) + maskShift, mountainsBlend);
    float const ridges = ridgeNoise(posOnUnitSphere, mountainsParameters, mountainSharpness) * moutainMask * mountainsBlend;

    // Ocean bed
    float oceanFloorShape = -oceanFloorDepth + perlin_noise * 0.15f;
    float continentShape = smoothMax(perlin_noise, oceanFloorShape, oceanFloorSmoothing);
    continentShape *= (continentShape < 0) ? 1 + oceanDepthMultiplier : 1;

    vec3 newPosition = radius * posOnUnitSphere * (1 + (ridges + continentShape) * 0.1f);
    return newPosition;
}

Planet::Planet(float r, GLuint shader) {
    radius = r;
    //m = mesh_primitive_sphere();
    m = createIcoSphere(radius, 100);
    visual = mesh_drawable(m, shader);
    visual.shading.color = { 0.4, 0.35, 0.25 };
    visual.shading.phong.specular = 0.0f;
    this->shader = shader;
}

void Planet::updatePlanetMesh() {
    for (int i = 0; i < m.position.size(); i++) {
        m.position[i] = getPlanetRadiusAt(m.position[i]);
    }
    m.compute_normal();

    visual.update_position(m.position);
    visual.update_normal(m.normal);
    visual.update_color(m.color);
}

void Planet::setCustomUniforms() {
    glUseProgram(shader);
    opengl_uniform(shader, "textureScale", textureScale);
    opengl_uniform(shader, "textureSharpness", textureSharpness);
    opengl_uniform(shader, "normalMapInfluence", normalMapInfluence, false);
}

void Planet::setTexture(GLuint texture) {
    visual.texture = texture;
}

