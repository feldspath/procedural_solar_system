#pragma once
#include "camera_fps.hpp"


class Player {
private:
	camera_fps* camera;
	float walkSpeed = 5.0f;
	float thrustForce = 500.0f; // Thrust force of the jetpack in N
	float mass = 100.0f;

	PhysicsComponent* physics = nullptr;

public:
	void bindCamera(camera_fps* camera);
	void init_physics();
	void update_position(vcl::vec3 direction);
};

void Player::bindCamera(camera_fps* camera) {
	this->camera = camera;
}

void Player::init_physics() {
	physics = PhysicsComponent::generatePhysicsComponent(mass, camera->position_camera);
}

void Player::update_position(vcl::vec3 direction) {
	vcl::vec3 dir = direction.x * camera->front() + direction.y * camera->right() + direction.z * camera->up();
	float dir_norm = vcl::norm(dir);
	if (dir_norm > 0.1f) {
		physics->add_force(thrustForce * dir / dir_norm);
	}
	camera->position_camera = physics->get_position();
}