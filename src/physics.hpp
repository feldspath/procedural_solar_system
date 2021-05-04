#pragma once

#include "vcl/vcl.hpp"
#include <vector>


struct PhysicsComponent {
    unsigned int id;
	vcl::vec3 position;
	vcl::vec3 velocity;
	float mass;

	static float const G;
	static float fixedDeltaTime;
	static float deltaTimeOffset;
    static unsigned int objectCount;
    static std::vector<PhysicsComponent> objects;
	

	PhysicsComponent() {}
	PhysicsComponent(float m, vcl::vec3 p = { 0, 0, 0 }, vcl::vec3 v = { 0, 0, 0 });
	void update(float deltaTime);

private:
	void singleUpdate();
};
