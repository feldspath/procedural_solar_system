#pragma once
#include "camera_fps.hpp"
#include "planet.hpp"
#include "physics.hpp"


class Player {
private:
	camera_fps* camera;
	float walkSpeed = 0.8f;
	float thrustForce = 2000.0f; // Thrust force of the jetpack in N
	float height = 0.3f;
	bool onGround = false;
	bool jetpackOn = true;

	int currentPlanet = -1;

	std::vector<Planet>* planets;


public:
	float mass = 100.0f;
	vcl::vec3 currentSpeed = vcl::vec3(0.0f, 0.0f, 0.0f);
	vcl::vec3 additionalSpeed = vcl::vec3(0.0f, 0.0f, 0.0f);
	vcl::vec3 nextForce = vcl::vec3(0.0f, 0.0f, 0.0f);
	vcl::vec3 accel = vcl::vec3(0.0f, 0.0f, 0.0f);

	void bind_camera(camera_fps* camera);
	void bind_planets(std::vector<Planet>* planets);
	void init_physics();
	void update_position(vcl::int3 direction);
	void clamp_to_planets();
	void toggleJetpack();
};