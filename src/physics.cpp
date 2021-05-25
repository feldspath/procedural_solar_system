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
    nextPosition = p;
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
    float alpha = deltaTimeOffset / fixedDeltaTime;
    std::cout << deltaTimeOffset << std::endl;
    return nextPosition * (1-alpha) + position * alpha;

    //return position;
}


void PhysicsComponent::update(float deltaTime) {
    int updateCount = 0;
    float timeSpent = deltaTimeOffset;
    while (timeSpent < deltaTime) {
        timeSpent += fixedDeltaTime;
        singleUpdate();
        updateCount++;
    }
    deltaTimeOffset = timeSpent - deltaTime;
    //std::cout << updateCount << std::endl;
    

    for (int i = 0; i < objects.size(); i++)
        objects[i]->nextForce = vcl::vec3(0.0f, 0.0f, 0.0f);
}

void PhysicsComponent::singleUpdate() {
    int n = objects.size();

    for (int i = 0; i < n; i++) {
        objects[i]->position = objects[i]->nextPosition;
    }

    for (int i = 0; i < n; i++) {
        PhysicsComponent* current = objects[i];
        vcl::vec3 accel = { 0, 0, 0 };
        for (int j = 0; j < n; j++) {
            PhysicsComponent* obj = objects[j];
            if (i != j) {
                float sqrDist = (current->position.x - obj->position.x) * (current->position.x - obj->position.x);
                sqrDist += (current->position.y - obj->position.y) * (current->position.y - obj->position.y);
                sqrDist += (current->position.z - obj->position.z) * (current->position.z - obj->position.z);
                accel -= obj->mass / sqrDist * vcl::normalize(current->position - obj->position);
            }
        }
        accel *= G;
        accel += current->nextForce / current->mass;
        //if (i == 0) {
        //    std::cout << forces.x << "," << forces.y << "," << forces.z << std::endl;
        //    std::cout << "velocity : " << current->velocity.x << ',' << current->velocity.y << ',' << current->velocity.z << std::endl;
        //}
        //current->velocity += accel * fixedDeltaTime;
        current->velocity += accel * fixedDeltaTime;
        current->nextPosition = current->position + current->velocity * fixedDeltaTime;
        
    }    
}
