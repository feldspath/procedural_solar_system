#pragma once

#include "vcl/vcl.hpp"
#include "noises.hpp"
#include "mesh_drawable_multitexture.hpp"
#include "physics.hpp"

class Planet {

private:

    // Physics
    PhysicsComponent* physics = nullptr;

    // Rendering
    vcl::mesh m;
    vcl::mesh mLowRes;

public:
    vcl::mesh_drawable visual;
    vcl::mesh_drawable visualLowRes;

private:
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

    // Atmosphere
    bool hasAtmosphere = false;
    float atmosphereHeight = 3.0f;
    float densityFalloff = 3.0f;

    float wavelengths[3] = { 700, 530, 440 };
    float scatteringStrength = 1.0f;

    static int nScatteringPoints;
    static int nOpticalDepthPoints;

    // Misc
    bool isSun = false;
    bool waterGlow = false;
    bool specularWater = true;

    static float nearPlane;
    static float interPlane;
    static float farPlane;

    // Constructors
    Planet() {}
    Planet(char* name, float mass, vcl::vec3 position, vcl::vec3 velocity = {0, 0, 0}, int division=200);
    Planet(char* name, float mass, Planet* parent, float distanceToParent, float phase, int division = 200);

    // Getters
    vcl::vec3 getPosition();
    vcl::vec3 getSpeed();

    // Update functions
    vcl::vec3 getPlanetRadiusAt(const vcl::vec3& posOnUnitSphere);
    void updateFragmentMesh(vcl::uint2 division);
	void updatePlanetMesh();
    void updateVisual();
	void updateRotation(float deltaTime);

    // Rendering
    void displayInterface();
    
    void setCustomUniforms();
    template <typename SCENE> void renderPlanet(SCENE const& scene, bool lowRes=false);
    template <typename SCENE> void renderWater(SCENE const& scene);

    // Post processing
    static void initPlanetRenderer(const unsigned int width, const unsigned int height);
    static void buildTextures(const unsigned int width, const unsigned int height);
    static void startPlanetRendering();
    static void switchIntermediateTexture();
    static void renderFinalPlanet();
    template <typename SCENE> static void startWaterRendering(SCENE const& scene, bool nearPlanets);

private:
    static void buildFbo(const unsigned int width, const unsigned int height);

public:

    void exportToFile(const char* path);
    void importFromFile(const char* path);
	

};

template <typename SCENE>
void Planet::renderPlanet(SCENE const& scene, bool lowRes) {
    setCustomUniforms();
    visual.transform.translate = physics->get_position();
    visualLowRes.transform.translate = physics->get_position();
    if (!lowRes)
        vcl::draw(visual, scene);
    else
        vcl::draw(visualLowRes, scene);
}

template <typename SCENE>
void Planet::renderWater(SCENE const& scene) {
    vcl::vec4 center = vcl::vec4(visual.transform.translate, 1.0f);
    visual.transform.translate = physics->get_position();
    vcl::opengl_uniform(postProcessingQuad.shader, "worldPlanetCenter", center);
    vcl::opengl_uniform(postProcessingQuad.shader, "oceanLevel", waterLevel * radius);
    vcl::opengl_uniform(postProcessingQuad.shader, "depthMultiplier", depthMultiplier);
    vcl::opengl_uniform(postProcessingQuad.shader, "waterBlendMultiplier", waterBlendMultiplier);
    vcl::opengl_uniform(postProcessingQuad.shader, "waterColorDeep", waterColorDeep);
    vcl::opengl_uniform(postProcessingQuad.shader, "waterColorSurface", waterColorSurface);
    vcl::opengl_uniform(postProcessingQuad.shader, "lightSource", scene.light);
    vcl::opengl_uniform(postProcessingQuad.shader, "hasAtmosphere", hasAtmosphere);
    if (hasAtmosphere) {
        vcl::opengl_uniform(postProcessingQuad.shader, "planetRadius", radius);
        vcl::opengl_uniform(postProcessingQuad.shader, "atmosphereHeight", atmosphereHeight * radius);
        vcl::opengl_uniform(postProcessingQuad.shader, "densityFalloff", densityFalloff);

        vcl::vec3 scatteringCoeffs;
        scatteringCoeffs.x = std::pow(50 / wavelengths[0], 4) * scatteringStrength;
        scatteringCoeffs.y = std::pow(50 / wavelengths[1], 4) * scatteringStrength;
        scatteringCoeffs.z = std::pow(50 / wavelengths[2], 4) * scatteringStrength;
        vcl::opengl_uniform(postProcessingQuad.shader, "scatteringCoeffs", scatteringCoeffs);
    }
    vcl::opengl_uniform(postProcessingQuad.shader, "isSun", isSun);
    vcl::opengl_uniform(postProcessingQuad.shader, "waterGlow", waterGlow);
    vcl::opengl_uniform(postProcessingQuad.shader, "specularWater", specularWater);
    
    draw(postProcessingQuad, scene);
}

template <typename SCENE>
void Planet::startWaterRendering(SCENE const& scene, bool nearPlanets) {
    glDisable(GL_DEPTH_TEST);

    glUseProgram(postProcessingQuad.shader);

    // Matrices
    vcl::opengl_uniform(postProcessingQuad.shader, "viewMatrix", scene.camera.matrix_view());
    vcl::opengl_uniform(postProcessingQuad.shader, "perspectiveInverse", inverse(scene.projection));
    
    if (nearPlanets) {
        vcl::opengl_uniform(postProcessingQuad.shader, "near", nearPlane);
        vcl::opengl_uniform(postProcessingQuad.shader, "far", interPlane);
    }
    else {
        vcl::opengl_uniform(postProcessingQuad.shader, "near", interPlane);
        vcl::opengl_uniform(postProcessingQuad.shader, "far", farPlane);
    }
    vcl::opengl_uniform(postProcessingQuad.shader, "nScatteringPoints", nScatteringPoints);
    vcl::opengl_uniform(postProcessingQuad.shader, "nOpticalDepthPoints", nOpticalDepthPoints);

    

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}
