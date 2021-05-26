#pragma once
#include "camera_fps.hpp"
#include "planet.hpp"
#include "physics.hpp"


class Player {
private:
	camera_fps* camera;
	float walkSpeed = 0.6f;
	float thrustForce = 500.0f; // Thrust force of the jetpack in N
	float mass = 100.0f;
	float height = 0.2f;

	int currentPlanet = -1;

	PhysicsComponent* physics = nullptr;

	std::vector<Planet>* planets;


public:
	void bind_camera(camera_fps* camera);
	void bind_planets(std::vector<Planet>* planets);
	void init_physics();
	void update_position(vcl::vec3 direction);
	void clamp_to_planets();
};