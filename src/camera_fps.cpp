#include "camera_fps.hpp"

void camera_fps::update_orientation(vcl::vec2 mouse_offset) {
	manipulator_rotate_roll_pitch_yaw(0, mouse_offset.y * sensitivity, mouse_offset.x * sensitivity);
}