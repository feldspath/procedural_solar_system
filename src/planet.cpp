#include "planet.hpp"
#include "icosphere.hpp"
#include "vcl/vcl.hpp"
#include "noises.hpp"
#include "mesh_drawable_multitexture.hpp"

using namespace vcl;

// Helper functions
static float smoothMax(float a, float b, float k) {
    return std::log(std::exp(a * k) + std::exp(b * k)) / k;
}

static float blend(float noise, float blending) {
    return std::atan(noise * blending) / pi + 0.5f;
}

// Planet members declaration
mesh_drawable_multitexture Planet::postProcessingQuad;
GLuint Planet::shader = -1;
GLuint Planet::fbo;
GLuint Planet::depth_buffer;
GLuint Planet::intermediate_image;
GLuint Planet::intermediate_image_bis;
bool Planet::base_intermediate_image;

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

Planet::Planet(float r, float mass, vcl::vec3 position, vcl::vec3 velocity, int division) {
    radius = r;
    //m = mesh_primitive_sphere();
    m = mesh_icosphere(radius, division);

    visual = mesh_drawable(m, shader);
    visual.shading.color = { 0.4, 0.35, 0.25 };
    visual.shading.phong.specular = 0.0f;
    updatePlanetMesh();

    //image_raw const im = image_load_png("assets/checker_texture.png");
    image_raw const im = image_load_png("assets/moon_normal_map1.png");
    GLuint const planetTexture = opengl_texture_to_gpu(im, GL_REPEAT, GL_REPEAT);
    visual.texture = planetTexture;
    

    physics = PhysicsComponent::generatePhysicsComponent(mass, position, velocity);
}

void Planet::updatePlanetMesh() {
    for (int i = 0; i < (int)m.position.size(); i++) {
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

void Planet::updateRotation(float deltaTime) {
    vcl::rotation rot({ 0.0f, 0.0f, 1.0f }, deltaTime * rotateSpeed);
    visual.transform.rotate = rot * visual.transform.rotate;
}

void Planet::initPlanetRenderer(const unsigned int width, const unsigned int height) {
    // Planet shader
    shader = opengl_create_shader_program(read_text_file(("shaders/planet/planet.vert.glsl")), read_text_file("shaders/planet/planet.frag.glsl"));

    // Fbo
    buildFbo(width, height);

    // Post processing quad
    postProcessingQuad = mesh_drawable_multitexture(mesh_primitive_quadrangle({ -1,-1,0 }, { 1,-1,0 }, { 1,1,0 }, { -1,1,0 }));
    postProcessingQuad.texture = intermediate_image;
    postProcessingQuad.texture_2 = depth_buffer;

    GLuint const shader_screen_render = opengl_create_shader_program(read_text_file("shaders/planet/water.vert.glsl"), read_text_file("shaders/planet/water.frag.glsl"));
    postProcessingQuad.shader = shader_screen_render;
    buildTextures(width, height);
}

void Planet::buildFbo(const unsigned int width, const unsigned int height) {
    glGenFramebuffers(1, &fbo);
    glGenTextures(1, &depth_buffer);
    glGenTextures(1, &intermediate_image);
    glGenTextures(1, &intermediate_image_bis);

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

    // Texture color buffer
    glBindTexture(GL_TEXTURE_2D, intermediate_image_bis);
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

void Planet::startPlanetRendering() {
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, intermediate_image, 0);
    base_intermediate_image = true;
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Planet::switchIntermediateTexture() {
    if (base_intermediate_image) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, intermediate_image_bis, 0);
        postProcessingQuad.texture = intermediate_image;
    }
    else {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, intermediate_image, 0);
        postProcessingQuad.texture = intermediate_image_bis;
    }
    base_intermediate_image = !base_intermediate_image;
}

void Planet::renderFinalPlanet() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (base_intermediate_image)
        postProcessingQuad.texture = intermediate_image;
    else
        postProcessingQuad.texture = intermediate_image_bis;
}

void Planet::displayInterface() {
    bool update = false;
    if (ImGui::CollapsingHeader("Planet parameters")) {

        ImGui::SliderFloat("Rotation speed", &rotateSpeed, -5.0f, 5.0f);

        float col[3] = { visual.shading.color.x,visual.shading.color.y , visual.shading.color.z };
        ImGui::ColorEdit3("Planet color", col);
        visual.shading.color = vec3(col[0], col[1], col[2]);


        if (ImGui::TreeNode("Terrain generation")) {
            update |= ImGui::SliderFloat("Radius", &radius, 0.0f, 3.0f);
            if (ImGui::TreeNode("Continent noise")) {
                update |= displayPerlinNoiseGui(continentParameters);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Mountain noise")) {
                update |= ImGui::SliderFloat("Sharpness", &mountainSharpness, 0.1f, 5.0f);
                update |= displayPerlinNoiseGui(mountainsParameters);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Oceans")) {
                update |= ImGui::SliderFloat("Floor depth", &oceanFloorDepth, 0.0f, 1.0f);
                update |= ImGui::SliderFloat("Floor smoothing", &oceanFloorSmoothing, 0.1f, 20.0f);
                update |= ImGui::SliderFloat("Depth multiplier", &oceanDepthMultiplier, 0.0f, 10.0f);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Moutains Mask")) {
                update |= ImGui::SliderFloat("Moutains blend", &mountainsBlend, 0.1f, 20.0f);
                update |= ImGui::SliderFloat("Vertical shift", &maskShift, -2.0f, 2.0f);
                update |= displayPerlinNoiseGui(maskParameters);
                ImGui::TreePop();
            }

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Water")) {
            ImGui::SliderFloat("Water level", &waterLevel, 0.0f, 3.0f);

            // Water Color
            float colD[3] = { waterColorSurface.x, waterColorSurface.y , waterColorSurface.z };
            ImGui::ColorEdit3("Surface water", colD);
            waterColorSurface = vec3(colD[0], colD[1], colD[2]);
            float colS[3] = { waterColorDeep.x, waterColorDeep.y , waterColorDeep.z };
            ImGui::ColorEdit3("Deep water", colS);
            waterColorDeep = vec3(colS[0], colS[1], colS[2]);

            ImGui::SliderFloat("Depth multiplier", &depthMultiplier, 0.0f, 10.0f);
            ImGui::SliderFloat("Water blend multipler", &waterBlendMultiplier, 0.0f, 100.0f);
            ImGui::TreePop();

        }
        if (ImGui::TreeNode("Texture")) {
            ImGui::SliderFloat("Scale", &textureScale, 0.1f, 2.0f);
            ImGui::SliderFloat("Sharpness", &textureSharpness, 0.1f, 5.0f);
            ImGui::SliderFloat("Normal map influence", &normalMapInfluence, 0.0f, 1.0f);
            ImGui::TreePop();
        }
    }

    if (update)
        updatePlanetMesh();
}