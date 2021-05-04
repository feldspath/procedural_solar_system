#pragma once

#include "vcl/vcl.hpp"
#include <vector>


struct PhysicsComponent {
	vcl::vec3 position;
	vcl::vec3 velocity;
	float mass;

	static float const G;
	static float fixedDeltaTime;
	static float deltaTimeOffset;
    static std::vector<PhysicsComponent> objects;
	

	PhysicsComponent() {}
	PhysicsComponent(float m, vcl::vec3 p = { 0, 0, 0 }, vcl::vec3 v = { 0, 0, 0 });

    static int generatePhysicsComponent(float m, vcl::vec3 p = { 0, 0, 0 }, vcl::vec3 v = { 0, 0, 0 });
    static void update(float deltaTime);

private:
    static void singleUpdate();
};
