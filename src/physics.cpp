#include "physics.hpp"
#include "vcl/vcl.hpp"
#include <vector>
#include <iostream>

float const PhysicsComponent::G = 6.67430e-11;
float PhysicsComponent::fixedDeltaTime = 0.02f;
float PhysicsComponent::deltaTimeOffset;
std::vector<PhysicsComponent> PhysicsComponent::objects;

PhysicsComponent::PhysicsComponent(float m, vcl::vec3 p, vcl::vec3 v) {
	mass = m;
	position = p;
	velocity = v;
}

int PhysicsComponent::generatePhysicsComponent(float m, vcl::vec3 p, vcl::vec3 v) {
    objects.push_back(PhysicsComponent(m, p, v));
    return objects.size()-1;
}


void PhysicsComponent::update(float deltaTime) {
	float timeInterval = deltaTime + deltaTimeOffset;
	int n = timeInterval / fixedDeltaTime;
	deltaTimeOffset = timeInterval - n * fixedDeltaTime;
	for (int i = 0; i < n; i++)
		singleUpdate();
}

void PhysicsComponent::singleUpdate() {
    int n = objects.size();
    std::vector<vcl::vec3> newPositions;
    newPositions.resize(n);
    for (int i = 0; i < n; i++) {
        PhysicsComponent& current = objects.at(i);
        vcl::vec3 forces = { 0, 0, 0 };
        for (int j = 0; j < n; j++) {
            PhysicsComponent& obj = objects.at(j);
            if (i != j) {
                float sqrDist = (current.position.x - obj.position.x) * (current.position.x - obj.position.x);
                sqrDist += (current.position.y - obj.position.y) * (current.position.y - obj.position.y);
                sqrDist += (current.position.z - obj.position.z) * (current.position.z - obj.position.z);
                forces -= obj.mass / sqrDist * vcl::normalize(current.position - obj.position);
            }
        }
        forces *= G;
        current.velocity += forces * fixedDeltaTime;
        newPositions[i] = current.velocity * fixedDeltaTime;
    }

    for (int i = 0; i < n; i++) {
        objects.at(i).position += newPositions[i];
    }
}
