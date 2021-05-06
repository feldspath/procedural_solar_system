#pragma once

#include "vcl/vcl.hpp"
#include "noises.hpp"
#include "mesh_drawable_multitexture.hpp"
#include "physics.hpp"

class Planet {

private:

	vcl::mesh m;
    static GLuint shader;

    // Physics
    unsigned int physics;

public:

	vcl::mesh_drawable visual;

    static mesh_drawable_multitexture postProcessingQuad;
    static GLuint fbo;                // Frame buffer for multi-pass render
    static GLuint depth_buffer;       // Depth buffer used when rendered in the frame buffer
    static GLuint intermediate_image; // Texture of the rendered color image

	float radius = 1.0f;
	float rotateSpeed = 0.0f;

	// Continent
	perlin_noise_parameters continentParameters;

	// Mountains
	perlin_noise_parameters mountainsParameters;
	perlin_noise_parameters maskParameters;

	float mountainSharpness = 1.0f;
	float mountainsBlend = 1.0f;
	float maskShift = 0.0f;

	// Oceans
	float oceanFloorDepth = 0.5f;
	float oceanFloorSmoothing = 1.0f;
	float oceanDepthMultiplier = 2.0f;

    // Water
    float waterLevel = 1.0f;
    vcl::vec3 waterColorSurface = { 0.0f, 0.2f, 1.0f };
    vcl::vec3 waterColorDeep = { 0.0f, 0.0f, 0.0f };
    float depthMultiplier = 6.0f;
    float waterBlendMultiplier = 60.0f;

	// Texture
	float textureScale = 1.0f;
	float textureSharpness = 5.0f;
	float normalMapInfluence = 0.2f;

	Planet() {}

    Planet(float r, float mass, vcl::vec3 position, vcl::vec3 velocity = {0, 0, 0}, int division=200);

	template <typename SCENE>
    void renderPlanet(SCENE const& scene);

    template <typename SCENE>
    void renderWater(SCENE const& scene);

	void updatePlanetMesh();

	void setCustomUniforms();

	void updateRotation(float deltaTime);

	vcl::vec3 getPlanetRadiusAt(vcl::vec3& position);
    void Planet::displayInterface();

    static void initPlanetRenderer(const unsigned int width, const unsigned int height);
    static void buildTextures(const unsigned int width, const unsigned int height);

    static void startPlanetRendering();

    template <typename SCENE>
    static void startWaterRendering(SCENE const& scene);

private:
    static void buildFbo(const unsigned int width, const unsigned int height);
	

};

template <typename SCENE>
void Planet::renderPlanet(SCENE const& scene) {
    setCustomUniforms();
    visual.transform.translate = PhysicsComponent::objects.at(physics).position;
    draw(visual, scene);
}

template <typename SCENE>
void Planet::renderWater(SCENE const& scene) {
    vcl::vec4 center = vcl::vec4(visual.transform.translate, 1.0f);
    opengl_uniform(postProcessingQuad.shader, "planetCenter", center);
    opengl_uniform(postProcessingQuad.shader, "oceanLevel", waterLevel);
    opengl_uniform(postProcessingQuad.shader, "depthMultiplier", depthMultiplier);
    opengl_uniform(postProcessingQuad.shader, "waterBlendMultiplier", waterBlendMultiplier);
    opengl_uniform(postProcessingQuad.shader, "waterColorDeep", waterColorDeep);
    opengl_uniform(postProcessingQuad.shader, "waterColorSurface", waterColorSurface);
    draw(postProcessingQuad, scene);
}

template <typename SCENE>
void Planet::startWaterRendering(SCENE const& scene) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(postProcessingQuad.shader);

    // Matrices
    opengl_uniform(postProcessingQuad.shader, "viewMatrix", scene.camera.matrix_view(), true);
    opengl_uniform(postProcessingQuad.shader, "perspectiveInverse", inverse(scene.projection), true);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}
