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
    float const perlin_noise = perlinNoise(posOnUnitSphere, continentParameters);
    
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

Planet::Planet(float r, const unsigned int width, const unsigned int height, int division) {
    radius = r;
    //m = mesh_primitive_sphere();
    m = mesh_icosphere(radius, division);

    shader = opengl_create_shader_program(read_text_file(("shaders/planet/planet.vert.glsl")), read_text_file("shaders/planet/planet.frag.glsl"));
    this->shader = shader;
    visual = mesh_drawable(m, shader);
    visual.shading.color = { 0.4, 0.35, 0.25 };
    visual.shading.phong.specular = 0.0f;

    //image_raw const im = image_load_png("assets/checker_texture.png");
    image_raw const im = image_load_png("assets/moon_normal_map1.png");
    GLuint const planetTexture = opengl_texture_to_gpu(im, GL_REPEAT, GL_REPEAT);
    visual.texture = planetTexture;
    updatePlanetMesh();


    buildFbo(width, height);

    postProcessingQuad = mesh_drawable_multitexture(mesh_primitive_quadrangle({ -1,-1,0 }, { 1,-1,0 }, { 1,1,0 }, { -1,1,0 }));
    postProcessingQuad.texture = intermediate_image;
    postProcessingQuad.texture_2 = depth_buffer;

    GLuint const shader_screen_render = opengl_create_shader_program(read_text_file("shaders/planet/water.vert.glsl"), read_text_file("shaders/planet/water.frag.glsl"));
    postProcessingQuad.shader = shader_screen_render;

    
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

void Planet::rotatePlanet(float deltaTime) {
    vcl::rotation rot({ 0.0f, 0.0f, 1.0f }, deltaTime * rotateSpeed);
    visual.transform.rotate = rot * visual.transform.rotate;
}

void Planet::buildFbo(const unsigned int width, const unsigned int height) {
    glGenFramebuffers(1, &fbo);
    glGenTextures(1, &depth_buffer);
    glGenTextures(1, &intermediate_image);

    buildTextures(width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, intermediate_image, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_buffer, 0);
    opengl_check;

    assert_vcl(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Error : Framebuffer is not complete");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void Planet::buildTextures(const unsigned int width, const unsigned int height) {
    // Texture color buffer
    glBindTexture(GL_TEXTURE_2D, intermediate_image);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    opengl_check;

    // Texture depth buffer
    glBindTexture(GL_TEXTURE_2D, depth_buffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    opengl_check;
}


