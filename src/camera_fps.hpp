#pragma once

#include "vcl/interaction/camera/camera.hpp"
#include "physics.hpp"
#include "planet.hpp"


class camera_fps : public vcl::camera_head {

private:
	float sensitivity=1.0f;

public:
	void update_orientation(vcl::vec2 mouse_offset);
};

void camera_fps::update_orientation(vcl::vec2 mouse_offset) {
	manipulator_rotate_roll_pitch_yaw(0, mouse_offset.y * sensitivity, mouse_offset.x * sensitivity);
}

