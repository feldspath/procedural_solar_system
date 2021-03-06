#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>

#include "planet.hpp"
#include "icosphere.hpp"
#include "vcl/vcl.hpp"
#include "noises.hpp"
#include "mesh_drawable_multitexture.hpp"

#define N_THREADS 5

using namespace vcl;

// Helper functions
static float smoothMax(float a, float b, float k) {
    return std::log(std::exp(a * k) + std::exp(b * k)) / k;
}

static float blend(float noise, float blending) {
    return std::atan(noise * blending) / pi + 0.5f;
}

static std::vector<size_t> importerLookupTable;
static std::vector<char> importerMemberSizes;

static bool little_endian;

static void buildImporterLookupTable() {
    importerLookupTable.resize(0);
    importerLookupTable.push_back(offsetof(Planet, radius));
    importerLookupTable.push_back(offsetof(Planet, rotateSpeed));
    importerLookupTable.push_back(offsetof(Planet, flatLowColor));
    importerLookupTable.push_back(offsetof(Planet, flatHighColor));
    importerLookupTable.push_back(offsetof(Planet, steepColor));
    importerLookupTable.push_back(offsetof(Planet, maxSlope));
    importerLookupTable.push_back(offsetof(Planet, continentParameters));
    importerLookupTable.push_back(offsetof(Planet, mountainsParameters));
    importerLookupTable.push_back(offsetof(Planet, maskParameters));
    importerLookupTable.push_back(offsetof(Planet, mountainSharpness));
    importerLookupTable.push_back(offsetof(Planet, mountainsBlend));
    importerLookupTable.push_back(offsetof(Planet, maskShift));
    importerLookupTable.push_back(offsetof(Planet, oceanFloorDepth));
    importerLookupTable.push_back(offsetof(Planet, oceanFloorSmoothing));
    importerLookupTable.push_back(offsetof(Planet, oceanDepthMultiplier));
    importerLookupTable.push_back(offsetof(Planet, waterLevel));
    importerLookupTable.push_back(offsetof(Planet, waterColorSurface));
    importerLookupTable.push_back(offsetof(Planet, waterColorDeep));
    importerLookupTable.push_back(offsetof(Planet, depthMultiplier));
    importerLookupTable.push_back(offsetof(Planet, waterBlendMultiplier));
    importerLookupTable.push_back(offsetof(Planet, textureScale));
    importerLookupTable.push_back(offsetof(Planet, textureSharpness));
    importerLookupTable.push_back(offsetof(Planet, normalMapInfluence));
    importerLookupTable.push_back(offsetof(Planet, hasAtmosphere));
    importerLookupTable.push_back(offsetof(Planet, atmosphereHeight));
    importerLookupTable.push_back(offsetof(Planet, densityFalloff));
    importerLookupTable.push_back(offsetof(Planet, wavelengths));
    importerLookupTable.push_back(offsetof(Planet, scatteringStrength));

    importerMemberSizes.push_back(sizeof(Planet::radius));
    importerMemberSizes.push_back(sizeof(Planet::rotateSpeed));
    importerMemberSizes.push_back(sizeof(Planet::flatLowColor));
    importerMemberSizes.push_back(sizeof(Planet::flatHighColor));
    importerMemberSizes.push_back(sizeof(Planet::steepColor));
    importerMemberSizes.push_back(sizeof(Planet::maxSlope));
    importerMemberSizes.push_back(sizeof(Planet::continentParameters));
    importerMemberSizes.push_back(sizeof(Planet::mountainsParameters));
    importerMemberSizes.push_back(sizeof(Planet::maskParameters));
    importerMemberSizes.push_back(sizeof(Planet::mountainSharpness));
    importerMemberSizes.push_back(sizeof(Planet::mountainsBlend));
    importerMemberSizes.push_back(sizeof(Planet::maskShift));
    importerMemberSizes.push_back(sizeof(Planet::oceanFloorDepth));
    importerMemberSizes.push_back(sizeof(Planet::oceanFloorSmoothing));
    importerMemberSizes.push_back(sizeof(Planet::oceanDepthMultiplier));
    importerMemberSizes.push_back(sizeof(Planet::waterLevel));
    importerMemberSizes.push_back(sizeof(Planet::waterColorSurface));
    importerMemberSizes.push_back(sizeof(Planet::waterColorDeep));
    importerMemberSizes.push_back(sizeof(Planet::depthMultiplier));
    importerMemberSizes.push_back(sizeof(Planet::waterBlendMultiplier));
    importerMemberSizes.push_back(sizeof(Planet::textureScale));
    importerMemberSizes.push_back(sizeof(Planet::textureSharpness));
    importerMemberSizes.push_back(sizeof(Planet::normalMapInfluence));
    importerMemberSizes.push_back(sizeof(Planet::hasAtmosphere));
    importerMemberSizes.push_back(sizeof(Planet::atmosphereHeight));
    importerMemberSizes.push_back(sizeof(Planet::densityFalloff));
    importerMemberSizes.push_back(sizeof(Planet::wavelengths));
    importerMemberSizes.push_back(sizeof(Planet::scatteringStrength));


    // Check wether or not the system is little endian
    union {
        uint32_t i;
        char c[4];
    } bint = { 0x01020304 };
    little_endian = (bint.c[0] != 1);
}

// Planet members declaration
mesh_drawable_multitexture Planet::postProcessingQuad;
GLuint Planet::shader = (GLuint)-1;
GLuint Planet::fbo;
GLuint Planet::depth_buffer;
GLuint Planet::intermediate_image;
GLuint Planet::intermediate_image_bis;
bool Planet::base_intermediate_image;
int Planet::nScatteringPoints = 15;
int Planet::nOpticalDepthPoints = 15;
float Planet::nearPlane;
float Planet::interPlane;
float Planet::farPlane;

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

    vec3 newPosition = radius * posOnUnitSphere * (1 + (ridges + continentShape) * 0.03f);
    return newPosition;
}



Planet::Planet(char* name, float mass, vcl::vec3 position, vcl::vec3 velocity, int division) {

    // Planet mesh
    //m = mesh_primitive_sphere();
    m = mesh_icosphere(radius, division);
    visual = mesh_drawable(m, shader);
    visual.shading.color = { 1.0f, 1.0f, 1.0f };
    visual.shading.phong.specular = 0.0f;
    visual.shading.phong.ambient = 0.01f;

    // Low res planet
    mLowRes = mesh_icosphere(radius, 100);
    visualLowRes = mesh_drawable(mLowRes, shader);
    visualLowRes.shading.color = { 1.0f, 1.0f, 1.0f };
    visualLowRes.shading.phong.specular = 0.0f;
    visualLowRes.shading.phong.ambient = 0.01f;
    
    
    std::string path = "planets/" + std::string(name) + ".pbf";
    importFromFile(path.c_str());
    //updatePlanetMesh();

    // Texture
    //image_raw const im = image_load_png("assets/checker_texture.png");
    image_raw const im = image_load_png("assets/moon_normal_map1.png");
    GLuint const planetTexture = opengl_texture_to_gpu(im, GL_REPEAT, GL_REPEAT);
    visual.texture = planetTexture;
    visualLowRes.texture = planetTexture;

    // Physics
    physics = PhysicsComponent::generatePhysicsComponent(mass, position, velocity);
}

Planet::Planet(char* name, float mass, Planet* parent, float distanceToParent, float phase, int division) {
    vcl::vec3 position = parent->getPosition() + distanceToParent * vcl::vec3(std::cos(phase), std::sin(phase), 0.0f);
    vcl::vec3 velocity = std::sqrt(PhysicsComponent::G * parent->physics->get_mass() / distanceToParent) * vcl::vec3(-std::sin(phase), std::cos(phase), 0.0f) + parent->getSpeed();
    *this = Planet(name, mass, position, velocity, division);
}



void Planet::updateFragmentMesh(vcl::uint2 division) {
    for (int i = division.x; i < division.y; i++) {
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

        float slopeEstimate = std::abs(norm(getPlanetRadiusAt(normalize(posOnUnitSphere + stepSize * direction))) - height) / stepSize;
        direction = cross(posOnUnitSphere, direction);
        slopeEstimate = std::max(slopeEstimate, std::abs(norm(getPlanetRadiusAt(normalize(posOnUnitSphere + stepSize * direction))) - height) / stepSize);
        float blending = std::min(slopeEstimate / (maxSlope * radius), 1.0f);
        m.color[i] = vec3(height / (2 * radius), blending, 0.0f);
    }
}



void Planet::updatePlanetMesh() {
    continentParameters.octave = (int)continentParameters.octave;
    mountainsParameters.octave = (int)mountainsParameters.octave;
    maskParameters.octave = (int)maskParameters.octave;

    int size = (int)m.position.size();
    int divisionSize = size / N_THREADS;
    std::vector<std::thread> threads(N_THREADS);
    int index = 0;
    for (int i = 0; i < N_THREADS; i++) {
        vcl::uint2 division;
        division.x = index;
        division.y = (i == N_THREADS-1 ? size : index + divisionSize);
        index = division.y;
        threads[i] = std::thread(&Planet::updateFragmentMesh, this, division);
    }
    for (int i = 0; i < N_THREADS; i++) {
        threads[i].join();
    }
    m.compute_normal();

    for (int i = 0; i < mLowRes.position.size(); i++) {
        // Position
        const vec3 posOnUnitSphere = normalize(mLowRes.position[i]);
        mLowRes.position[i] = getPlanetRadiusAt(posOnUnitSphere);
        float height = norm(mLowRes.position[i]);

        // Color
        float stepSize = 0.02f;
        vec3 direction;
        if (std::abs(posOnUnitSphere.z) < 1.0f - 0.00001f)
            direction = normalize(cross(posOnUnitSphere, vec3(0.0f, 0.0f, 1.0f)));
        else
            direction = normalize(cross(posOnUnitSphere, vec3((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX)));

        float slopeEstimate = std::abs(norm(getPlanetRadiusAt(normalize(posOnUnitSphere + stepSize * direction))) - height) / stepSize;
        direction = cross(posOnUnitSphere, direction);
        slopeEstimate = std::max(slopeEstimate, std::abs(norm(getPlanetRadiusAt(normalize(posOnUnitSphere + stepSize * direction))) - height) / stepSize);
        float blending = std::min(slopeEstimate / (maxSlope * radius), 1.0f);
        mLowRes.color[i] = vec3(height / (2 * radius), blending, 0.0f);
    }
    mLowRes.compute_normal();
}

void Planet::updateVisual() {
    visual.update_position(m.position);
    visual.update_normal(m.normal);
    visual.update_color(m.color);

    visualLowRes.update_position(mLowRes.position);
    visualLowRes.update_normal(mLowRes.normal);
    visualLowRes.update_color(mLowRes.color);
}

vcl::vec3 Planet::getPosition() {
    return physics->get_position();
}

vcl::vec3 Planet::getSpeed() {
    return physics->get_speed();
}

void Planet::setCustomUniforms() {
    glUseProgram(shader);
    opengl_uniform(shader, "textureScale", textureScale);
    opengl_uniform(shader, "textureSharpness", textureSharpness);
    opengl_uniform(shader, "normalMapInfluence", normalMapInfluence, false);
    opengl_uniform(shader, "steepColor", steepColor);
    opengl_uniform(shader, "flatLowColor", flatLowColor);
    opengl_uniform(shader, "flatHighColor", flatHighColor);
    opengl_uniform(shader, "isSun", isSun);
    
}

void Planet::updateRotation(float deltaTime) {
    vcl::rotation rot({ 0.0f, 0.0f, 1.0f }, deltaTime * rotateSpeed);
    visual.transform.rotate = rot * visual.transform.rotate;
    visualLowRes.transform.rotate = rot * visualLowRes.transform.rotate;
}

void Planet::exportToFile(const char* path) {
    if (!importerLookupTable.size())
        buildImporterLookupTable();

    std::ofstream file(path, std::ofstream::binary);
    if (!file.is_open()) {
        std::cerr << "ERROR : failed to open file at path " << path << std::endl;
        return;
    }
    std::cout << "Writing to file " << path << std::endl;
    
    char buffer[512];
    int idx = 0;
    for (int i = 0; i < importerLookupTable.size(); i++) {
        char size = importerMemberSizes[i];
        buffer[idx++] = size;
        for (int j = 0; j < size; j++)
            buffer[idx++] = *((char*)this + importerLookupTable[i] + j);
    }
    file.write(buffer, idx);
    file.close();
}

void Planet::importFromFile(const char* path) {
    if (!importerLookupTable.size())
        buildImporterLookupTable();

    std::ifstream file(path, std::ifstream::binary);
    if (!file.is_open()) {
        std::cerr << "ERROR : failed to read file at path " << path << std::endl;
        return;
    }
    file.seekg(0, file.end);
    int fileSize = file.tellg();
    file.seekg(0, file.beg);
    char* buffer = new char[fileSize];
    file.read(buffer, fileSize);
   
    int idx = 0;
    int i = 0;
    while (idx < fileSize) {
        char size = buffer[idx++];
        for (int j = 0; j < size; j++) {
            if (little_endian)
                *((char*)this + importerLookupTable[i] + j) = buffer[idx++];
            else
                std::cerr << "ERROR: BIG_ENDIAN NOT SUPPORTED\n";
        }
            
        i++;
    }
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
    glClear(GL_DEPTH_BUFFER_BIT);
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
    //glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT);
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

        ImGui::SliderFloat("Rotation speed", &rotateSpeed, -0.2f, 0.2f);

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

            update |= ImGui::SliderFloat("Max slope", &maxSlope, 0.0f, 10.0f);

            ImGui::TreePop();
        }


        if (ImGui::TreeNode("Terrain generation")) {
            update |= ImGui::SliderFloat("Radius", &radius, 0.0f, 300.0f);
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
                update |= ImGui::SliderFloat("Floor smoothing", &oceanFloorSmoothing, 1.0f, 20.0f);
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
            ImGui::SliderFloat("Water level", &waterLevel, 0.9f, 1.5f);

            // Water Color
            float colD[4] = { waterColorSurface.x, waterColorSurface.y , waterColorSurface.z, waterColorSurface.w };
            ImGui::ColorEdit4("Surface water", colD);
            waterColorSurface = vec4(colD[0], colD[1], colD[2], colD[3]);
            float colS[4] = { waterColorDeep.x, waterColorDeep.y , waterColorDeep.z, waterColorDeep.w };
            ImGui::ColorEdit4("Deep water", colS);
            waterColorDeep = vec4(colS[0], colS[1], colS[2], colS[3]);

            ImGui::SliderFloat("Depth multiplier", &depthMultiplier, 0.0f, 0.01f);
            ImGui::SliderFloat("Water blend multipler", &waterBlendMultiplier, 0.0f, 60.0f);
            ImGui::TreePop();

        }
        if (ImGui::TreeNode("Texture")) {
            ImGui::SliderFloat("Scale", &textureScale, 0.1f, 2.0f);
            ImGui::SliderFloat("Sharpness", &textureSharpness, 0.1f, 5.0f);
            ImGui::SliderFloat("Normal map influence", &normalMapInfluence, 0.0f, 1.0f);
            ImGui::TreePop();
        }

        
        if (ImGui::TreeNode("Atmosphere")) {
            ImGui::Checkbox("Atmosphere", &hasAtmosphere);
            ImGui::SliderFloat("Atmosphere radius", &atmosphereHeight, 1.0f, 3.0f);
            ImGui::SliderFloat("Density falloff", &densityFalloff, 0.0f, 10.0f);
            ImGui::SliderInt("Scattering points", &Planet::nScatteringPoints, 0, 20);
            ImGui::SliderInt("Optical depth points", &Planet::nOpticalDepthPoints, 0, 20);

            ImGui::SliderFloat3("Wave lengths", wavelengths, 400, 800);
            ImGui::SliderFloat("Scattering strength", &scatteringStrength, 0.0f, 10000.0f);
            ImGui::TreePop();
        }
    }

    ImGui::InputText("##Planet Name", planet_name, 30);
    if (ImGui::Button("Save"))
    {
        std::string path = "planets/" + std::string(planet_name) + ".pbf";
        exportToFile(path.c_str());
    }

    if (update) {
        updatePlanetMesh();
        updateVisual();
    }
}