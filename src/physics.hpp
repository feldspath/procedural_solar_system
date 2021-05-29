#pragma once

#include "vcl/vcl.hpp"
#include <vector>

class Player;

class PhysicsComponent {
private:
	vcl::vec3 position;
	vcl::vec3 nextPosition;
	vcl::vec3 velocity;
	float mass = 0.0f;
	vcl::vec3 nextForce;
	


	static float fixedDeltaTime;
	static float deltaTimeOffset;
	static std::vector<PhysicsComponent*> objects;
	

public:
	PhysicsComponent() {}
	PhysicsComponent(float m, vcl::vec3 p = { 0, 0, 0 }, vcl::vec3 v = { 0, 0, 0 });
	vcl::vec3 additionalSpeed = vcl::vec3(0.0f, 0.0f, 0.0f);

	void add_force(vcl::vec3 force);
	vcl::vec3 get_position();
	vcl::vec3 get_speed();
	float get_mass();
	void set_position(vcl::vec3 position);
	void set_speed(vcl::vec3 speed);

    static PhysicsComponent* generatePhysicsComponent(float m, vcl::vec3 p = { 0, 0, 0 }, vcl::vec3 v = { 0, 0, 0 });
	static void deleteAllPhysicsCompoenents();
    static void update(float deltaTime);

	static float const G;
	static PhysicsComponent null;
	static Player* player;

private:
    static void singleUpdate();
};
