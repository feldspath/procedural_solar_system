#pragma once

#include "vcl/interaction/camera/camera.hpp"

class camera_fps : public vcl::camera_head {

private:
	float sensitivity=1.0f;

public:
	void update_orientation(vcl::vec2 mouse_offset);
};
