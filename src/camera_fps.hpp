#include "vcl/interaction/camera/camera.hpp"

class camera_fps : public vcl::camera_head {

private:
	float sensitivity;
	float max_speed;

public:
	void update_orientation(vcl::vec2 mouse_offset);
	void update_position(vcl::vec3 direction);

};