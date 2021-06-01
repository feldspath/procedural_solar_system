#pragma once

#include "vcl/vcl.hpp"

void createPlant(vcl::hierarchy_mesh_drawable& hierarchy);
void plantAnimation(vcl::hierarchy_mesh_drawable& hierarchy, float t);
vcl::buffer<vcl::buffer<float>> plantSpawn(int nombrePousses, vcl::vec3 colorLow, vcl::vec3 colorHigh, float sizeMax);