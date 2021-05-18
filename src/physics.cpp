#include "physics.hpp"
#include "vcl/vcl.hpp"
#include <vector>
#include <iostream>

float const PhysicsComponent::G = (float)6.67430e-11;
float PhysicsComponent::fixedDeltaTime = 0.02f;
float PhysicsComponent::deltaTimeOffset;
std::vector<PhysicsComponent*> PhysicsComponent::objects;
PhysicsComponent PhysicsComponent::null;

PhysicsComponent::PhysicsComponent(float m, vcl::vec3 p, vcl::vec3 v) {
	mass = m;
	position = p;
	velocity = v;
    nextForce = vcl::vec3(0.0f, 0.0f, 0.0f);
}

PhysicsComponent* PhysicsComponent::generatePhysicsComponent(float m, vcl::vec3 p, vcl::vec3 v) {
    objects.push_back(new PhysicsComponent(m, p, v));
    return objects.back();
}

void PhysicsComponent::deleteAllPhysicsCompoenents() {
    for (PhysicsComponent* p : objects)
        delete p;
    objects.resize(0);
}

void PhysicsComponent::add_force(vcl::vec3 force) {
    nextForce += force;
}

vcl::vec3 PhysicsComponent::get_position() {
    return position;
}


void PhysicsComponent::update(float deltaTime) {
	float timeInterval = deltaTime + deltaTimeOffset;
	int n = timeInterval / fixedDeltaTime;
	deltaTimeOffset = timeInterval - n * fixedDeltaTime;
	for (int i = 0; i < n; i++)
		singleUpdate();
    for (int i = 0; i < objects.size(); i++)
        objects[i]->nextForce = vcl::vec3(0.0f, 0.0f, 0.0f);
}

void PhysicsComponent::singleUpdate() {
    int n = objects.size();
    std::vector<vcl::vec3> newPositions;
    newPositions.resize(n);
    for (int i = 0; i < n; i++) {
        PhysicsComponent* current = objects.at(i);
        vcl::vec3 forces = { 0, 0, 0 };
        for (int j = 0; j < n; j++) {
            PhysicsComponent* obj = objects.at(j);
            if (i != j) {
                float sqrDist = (current->position.x - obj->position.x) * (current->position.x - obj->position.x);
                sqrDist += (current->position.y - obj->position.y) * (current->position.y - obj->position.y);
                sqrDist += (current->position.z - obj->position.z) * (current->position.z - obj->position.z);
                forces -= obj->mass / sqrDist * vcl::normalize(current->position - obj->position);
            }
        }
        forces *= G;
        forces += current->nextForce;
        //if (i == 0) {
        //    std::cout << forces.x << "," << forces.y << "," << forces.z << std::endl;
        //    std::cout << "velocity : " << current->velocity.x << ',' << current->velocity.y << ',' << current->velocity.z << std::endl;
        //}
        current->velocity += forces * fixedDeltaTime;
        newPositions[i] = current->velocity * fixedDeltaTime;
        
    }

    for (int i = 0; i < n; i++) {
        objects.at(i)->position += newPositions[i];
    }
}
