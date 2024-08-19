#pragma once

#include "vcl/vcl.hpp"
#include "camera_fps.hpp"
#include "player.hpp"
#include "skybox.hpp"

#define CAMERA_TYPE 1
// 0 is edit mode
// 1 is explore mode

// default window size
const int SCR_WIDTH = 1280;
const int SCR_HEIGHT = 1024;

// resolution of the planets
// WARNING: requires a lot of memory and loading time
// on my computer:
// 500 is 10s
// 1000 is almost 60s
static int resolution = 500;


#if CAMERA_TYPE
static float midDistance = 30.0f;
#else
static float midDistance = 5.0f;
#endif

struct scene_environment
{
#if CAMERA_TYPE
    camera_fps camera;
#else
    vcl::camera_around_center camera;
#endif
    vcl::mat4 projection;
    vcl::mat4 nearProjection;
    vcl::mat4 farProjection;
    vcl::vec3 light;
    Player player;
    std::vector<Planet> planets;
    Skybox skybox;

    vcl::hierarchy_mesh_drawable plant;
    vcl::buffer<vcl::buffer<float>> plantInfos;
};

void buildFrustrsums(scene_environment& scene, unsigned int width, unsigned int height, float interPlane = midDistance);
void display_scene(scene_environment& scene, float time, float width, float height);
void display_interface(scene_environment& scene, int* planet_index);
