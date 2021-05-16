#pragma once

#include "vcl/vcl.hpp"
#include "noises.hpp"
#include "mesh_drawable_multitexture.hpp"
#include "physics.hpp"

class Planet {

private:

    // Physics
    unsigned int physics;

    // Rendering
    vcl::mesh m;
    vcl::mesh_drawable visual;
    static GLuint shader;

    // Post processing
    static mesh_drawable_multitexture postProcessingQuad;
    static GLuint fbo;                // Frame buffer for multi-pass render
    static GLuint depth_buffer;       // Depth buffer used when rendered in the frame buffer
    static GLuint intermediate_image; // Texture of the rendered color image
    static GLuint intermediate_image_bis;
    static bool base_intermediate_image;

public:
	float radius = 1.0f;
	float rotateSpeed = 0.0f;

    // Colors
    vcl::vec3 flatLowColor = { 1.0f, 1.0f, 1.0f };
    vcl::vec3 flatHighColor = { 0.8f, 1.0f, 1.0f };
    vcl::vec3 steepColor = { 0.4f, 0.2f, 0.0f };
    float maxSlope = 0.3f;

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
    vcl::vec4 waterColorSurface = { 0.0f, 0.2f, 1.0f, 1.0f};
    vcl::vec4 waterColorDeep = { 0.0f, 0.0f, 0.0f, 1.0f };
    float depthMultiplier = 6.0f;
    float waterBlendMultiplier = 60.0f;

	// Texture
	float textureScale = 1.0f;
	float textureSharpness = 5.0f;
	float normalMapInfluence = 0.2f;

    // Constructors
	Planet() {}
    Planet(float r, float mass, vcl::vec3 position, vcl::vec3 velocity = {0, 0, 0}, int division=200, bool update_now=true);

    // Getters
    vcl::vec3 getPosition();

    // Update functions
    vcl::vec3 getPlanetRadiusAt(const vcl::vec3& posOnUnitSphere);
	void updatePlanetMesh();
	void updateRotation(float deltaTime);

    // Rendering
    void Planet::displayInterface();
    
    void setCustomUniforms();
    template <typename SCENE> void renderPlanet(SCENE const& scene);
    template <typename SCENE> void renderWater(SCENE const& scene);

    // Post processing
    static void initPlanetRenderer(const unsigned int width, const unsigned int height);
    static void buildTextures(const unsigned int width, const unsigned int height);
    static void startPlanetRendering();
    static void switchIntermediateTexture();
    static void renderFinalPlanet();
    template <typename SCENE> static void startWaterRendering(SCENE const& scene);

private:
    static void buildFbo(const unsigned int width, const unsigned int height);

public:

    void exportToFile(const char* path);
    void importFromFile(const char* path);
	

};

template <typename SCENE>
void Planet::renderPlanet(SCENE const& scene) {
    setCustomUniforms();
    visual.transform.translate = PhysicsComponent::objects.at(physics).position;
    vcl::draw(visual, scene);
}

template <typename SCENE>
void Planet::renderWater(SCENE const& scene) {
    vcl::vec4 center = vcl::vec4(visual.transform.translate, 1.0f);
    vcl::opengl_uniform(postProcessingQuad.shader, "planetCenter", center);
    vcl::opengl_uniform(postProcessingQuad.shader, "oceanLevel", waterLevel * radius);
    vcl::opengl_uniform(postProcessingQuad.shader, "depthMultiplier", depthMultiplier);
    vcl::opengl_uniform(postProcessingQuad.shader, "waterBlendMultiplier", waterBlendMultiplier);
    vcl::opengl_uniform(postProcessingQuad.shader, "waterColorDeep", waterColorDeep);
    vcl::opengl_uniform(postProcessingQuad.shader, "waterColorSurface", waterColorSurface);
    draw(postProcessingQuad, scene);
}

template <typename SCENE>
void Planet::startWaterRendering(SCENE const& scene) {
    glDisable(GL_DEPTH_TEST);

    glUseProgram(postProcessingQuad.shader);

    // Matrices
    vcl::opengl_uniform(postProcessingQuad.shader, "viewMatrix", scene.camera.matrix_view(), true);
    vcl::opengl_uniform(postProcessingQuad.shader, "perspectiveInverse", inverse(scene.projection), true);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}
