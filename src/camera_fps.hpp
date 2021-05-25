#include "vcl/interaction/camera/camera.hpp"
#include "physics.hpp"


class camera_fps : public vcl::camera_head {

private:
	float sensitivity=1.0f;
	float walkSpeed=5.0f;
	float thrustForce = 5.0f;
	float mass=100.0f;


	PhysicsComponent* physics = nullptr;

public:
	void init_physics();
	void update_orientation(vcl::vec2 mouse_offset);
	void update_position(vcl::vec3 direction);

};

void camera_fps::init_physics() {
	physics = PhysicsComponent::generatePhysicsComponent(mass, position_camera);
}

void camera_fps::update_orientation(vcl::vec2 mouse_offset) {
	manipulator_rotate_roll_pitch_yaw(0, mouse_offset.y * sensitivity, mouse_offset.x * sensitivity);
}

void camera_fps::update_position(vcl::vec3 direction) {
	vcl::vec3 dir = direction.x * front() + direction.y * right() + direction.z * up();
	float dir_norm = vcl::norm(dir);
	if (dir_norm > 0.1f) {
		physics->add_force(thrustForce * dir / dir_norm);
	}
	position_camera = physics->get_position();
}