#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>

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

vec3 Planet::getPlanetRadiusAt(const vec3& posOnUnitSphere) {
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

Planet::Planet(float r, float mass, vcl::vec3 position, vcl::vec3 velocity, int division, bool update_now) {
    radius = r;
    //m = mesh_primitive_sphere();
    m = mesh_icosphere(radius, division);

    visual = mesh_drawable(m, shader);
    visual.shading.color = { 1.0f, 1.0f, 1.0f };
    visual.shading.phong.specular = 0.0f;
    if (update_now)
        updatePlanetMesh();

    //image_raw const im = image_load_png("assets/checker_texture.png");
    image_raw const im = image_load_png("assets/moon_normal_map1.png");
    GLuint const planetTexture = opengl_texture_to_gpu(im, GL_REPEAT, GL_REPEAT);
    visual.texture = planetTexture;

    visual.shading.phong.ambient = 0.01f;
    

    physics = PhysicsComponent::generatePhysicsComponent(mass, position, velocity);
}

void Planet::updatePlanetMesh() {
    continentParameters.octave = (int)continentParameters.octave;
    mountainsParameters.octave = (int)mountainsParameters.octave;
    maskParameters.octave = (int)maskParameters.octave;

    for (int i = 0; i < (int)m.position.size(); i++) {
        // Position
        const vec3 posOnUnitSphere = normalize(m.position[i]);
        m.position[i] = getPlanetRadiusAt(posOnUnitSphere);
        float height = norm(m.position[i]);

        // Color
        float stepSize = 0.02f;
        vec3 direction;
        if (std::abs(posOnUnitSphere.z) < 1.0f - 0.00001f)
            direction = normalize(cross(posOnUnitSphere, vec3(0.0f, 0.0f, 1.0f)));
        else
            direction = normalize(cross(posOnUnitSphere, vec3((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX)));
        
        float slopeEstimate = std::abs(norm(getPlanetRadiusAt(normalize(posOnUnitSphere + stepSize * direction))) - height)/stepSize ;
        direction = cross(posOnUnitSphere, direction);
        slopeEstimate = std::max(slopeEstimate, std::abs(norm(getPlanetRadiusAt(normalize(posOnUnitSphere + stepSize * direction))) - height) / stepSize);
        float blending = std::min(slopeEstimate / (maxSlope * radius), 1.0f);
        m.color[i] = vec3(height/(2*radius), blending, 0.0f);

    }
    m.compute_normal();

    visual.update_position(m.position);
    visual.update_normal(m.normal);
    visual.update_color(m.color);
}

vcl::vec3 Planet::getPosition() {
    return PhysicsComponent::objects[physics].position;
}

void Planet::setCustomUniforms() {
    glUseProgram(shader);
    opengl_uniform(shader, "textureScale", textureScale);
    opengl_uniform(shader, "textureSharpness", textureSharpness);
    opengl_uniform(shader, "normalMapInfluence", normalMapInfluence, false);
    opengl_uniform(shader, "steepColor", steepColor);
    opengl_uniform(shader, "flatLowColor", flatLowColor);
    opengl_uniform(shader, "flatHighColor", flatHighColor);
}

void Planet::updateRotation(float deltaTime) {
    vcl::rotation rot({ 0.0f, 0.0f, 1.0f }, deltaTime * rotateSpeed);
    visual.transform.rotate = rot * visual.transform.rotate;
}

static void writeValue(const vec3& val, size_t offset, std::ofstream& file) {
    file << offset << ':' << val.x << ',' << val.y << ',' << val.z << '\n';
}

static void writeValue(const float& val, size_t offset, std::ofstream& file) {
    file << offset << ':' << val << '\n';
}

static void writeValue(const perlin_noise_parameters& val, size_t offset, std::ofstream& file) {
    file << offset << ':'
         << val.persistency << ','
         << val.frequency_gain << ','
         << val.octave << ','
         << val.center[0] << ',' << val.center[1] << ',' << val.center[2]
         << '\n';
}

void Planet::exportToFile(const char* path) {
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "ERROR : failed to open file at path " << path << std::endl;
        return;
    }
    std::cout << "Writing to file " << path << std::endl;
    writeValue(radius,               offsetof(Planet, radius), file);
    writeValue(rotateSpeed,          offsetof(Planet, rotateSpeed), file);
    writeValue(flatLowColor,         offsetof(Planet, flatLowColor), file);
    writeValue(flatHighColor,        offsetof(Planet, flatHighColor), file);
    writeValue(steepColor,           offsetof(Planet, steepColor), file);
    writeValue(maxSlope,             offsetof(Planet, maxSlope), file);
    writeValue(continentParameters,  offsetof(Planet, continentParameters), file);
    writeValue(mountainsParameters,  offsetof(Planet, mountainsParameters), file);
    writeValue(maskParameters,       offsetof(Planet, maskParameters), file);
    writeValue(mountainSharpness,    offsetof(Planet, mountainSharpness), file);
    writeValue(mountainsBlend,       offsetof(Planet, mountainsBlend), file);
    writeValue(maskShift,            offsetof(Planet, maskShift), file);
    writeValue(oceanFloorDepth,      offsetof(Planet, oceanFloorDepth), file);
    writeValue(oceanFloorSmoothing,  offsetof(Planet, oceanFloorSmoothing), file);
    writeValue(oceanDepthMultiplier, offsetof(Planet, oceanDepthMultiplier), file);
    writeValue(waterLevel,           offsetof(Planet, waterLevel), file);
    writeValue(waterColorSurface,    offsetof(Planet, waterColorSurface), file);
    writeValue(waterColorDeep,       offsetof(Planet, waterColorDeep), file);
    writeValue(depthMultiplier,      offsetof(Planet, depthMultiplier), file);
    writeValue(waterBlendMultiplier, offsetof(Planet, waterBlendMultiplier), file);
    writeValue(textureScale,         offsetof(Planet, textureScale), file);
    writeValue(textureSharpness,     offsetof(Planet, textureSharpness), file);
    writeValue(normalMapInfluence,   offsetof(Planet, normalMapInfluence), file);

    file.close();
}

void Planet::importFromFile(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "ERROR : failed to read file at path " << path << std::endl;
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        std::vector<float> values;
        int idx = line.find_first_of(':');
        size_t offset = std::stoi(line.substr(0, idx));
        line = line.substr(idx+1);
        while (idx != std::string::npos) {
            idx = line.find_first_of(',');
            values.push_back(std::stof(line.substr(0, idx)));
            line = line.substr(idx + 1);
        }
        for (int i = 0; i < values.size(); i++) {
            //std::cout << values[i] << std::endl;
            *(float*)((char*)this + offset + i * sizeof(float)) = values[i];
        }
    }

    updatePlanetMesh();
}


// STATIC FUNCTIONS

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

static char planet_name[30];

void Planet::displayInterface() {
    bool update = false;
    if (ImGui::CollapsingHeader("Planet parameters")) {

        ImGui::SliderFloat("Rotation speed", &rotateSpeed, -5.0f, 5.0f);

        if (ImGui::TreeNode("Terrain Color")) {
            float col1[3] = { steepColor.x,steepColor.y , steepColor.z };
            ImGui::ColorEdit3("Steep color", col1);
            steepColor = vec3(col1[0], col1[1], col1[2]);

            float col2[3] = { flatLowColor.x,flatLowColor.y , flatLowColor.z };
            ImGui::ColorEdit3("Flat color low", col2);
            flatLowColor = vec3(col2[0], col2[1], col2[2]);

            float col3[3] = { flatHighColor.x,flatHighColor.y , flatHighColor.z };
            ImGui::ColorEdit3("Flat color high", col3);
            flatHighColor = vec3(col3[0], col3[1], col3[2]);

            update |= ImGui::SliderFloat("Max slope", &maxSlope, 0.0f, 3.0f);

            ImGui::TreePop();
        }


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

    ImGui::InputText("##Planet Name", planet_name, 30);
    if (ImGui::Button("Save"))
    {
        std::string path = "planets/" + std::string(planet_name) + ".txt";
        exportToFile(path.c_str());
    }

    if (update)
        updatePlanetMesh();
}