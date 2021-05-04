#include "physics.h"
#include "vcl/vcl.hpp"
#include <vector>
#include <iostream>

float const PhysicsComponent::G = 6.67430e-11;
float PhysicsComponent::fixedDeltaTime = 0.02f;
float PhysicsComponent::deltaTimeOffset;
std::vector<PhysicsComponent> PhysicsComponent::objects;
unsigned int PhysicsComponent::objectCount = 0;

PhysicsComponent::PhysicsComponent(float m, vcl::vec3 p, vcl::vec3 v) {
	mass = m;
	position = p;
	velocity = v;
    id = objectCount++;
    objects.push_back(*this);
}

void PhysicsComponent::update(float deltaTime) {
	float timeInterval = deltaTime + deltaTimeOffset;
	int n = timeInterval / fixedDeltaTime;
	deltaTimeOffset = timeInterval - n * fixedDeltaTime;
	for (int i = 0; i < n; i++)
		singleUpdate();
}

void PhysicsComponent::singleUpdate() {
	vcl::vec3 forces = { 0, 0, 0 };
    int count = 0;
    for (PhysicsComponent obj : objects) {
        if (obj.id != id) {
            count++;
            float const dist = vcl::norm(position - obj.position);
            forces -= obj.mass / (dist * dist) * vcl::normalize(position - obj.position);
		}
	}
    forces *= G;
    velocity += forces * fixedDeltaTime;
	position += velocity * fixedDeltaTime;
}
