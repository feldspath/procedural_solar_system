#include "physics.h"
#include "vcl/vcl.hpp"
#include <vector>

float const PhysicsComponent::G = 6.67430e-11;
float PhysicsComponent::fixedDeltaTime = 0.05f;
float PhysicsComponent::deltaTimeOffset;
std::vector<PhysicsComponent*> PhysicsComponent::objects;

PhysicsComponent::PhysicsComponent(float m, vcl::vec3 p, vcl::vec3 v) {
	mass = m;
	position = p;
	velocity = v;
	objects.push_back(this);
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
	for (PhysicsComponent* obj : objects) {
		if (obj != this) {
			float const dist = vcl::norm(position - obj->position);
			forces -= obj->mass / (dist * dist) * vcl::normalize(position - obj->position);
		}
	}
	forces *= mass * G;
	velocity += forces * fixedDeltaTime;
	position += velocity * fixedDeltaTime;

}