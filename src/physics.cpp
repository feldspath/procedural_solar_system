#include "physics.hpp"
#include "vcl/vcl.hpp"
#include <vector>
#include <iostream>
#include "player.hpp"

float const PhysicsComponent::G = (float)6.67430e-11;
float PhysicsComponent::fixedDeltaTime = 0.005f;
float PhysicsComponent::deltaTimeOffset;
std::vector<PhysicsComponent*> PhysicsComponent::objects;
PhysicsComponent PhysicsComponent::null;
Player* PhysicsComponent::player = nullptr;

PhysicsComponent::PhysicsComponent(float m, vcl::vec3 p, vcl::vec3 v) {
	mass = m;
	position = p;
    nextPosition = p;
	velocity = v;
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

vcl::vec3 PhysicsComponent::get_position() {
    float alpha = deltaTimeOffset / fixedDeltaTime;
    return nextPosition * (1 - alpha) + position * alpha;
}

vcl::vec3 PhysicsComponent::get_speed() {
    return velocity;
}

float PhysicsComponent::get_mass() {
    return mass;
}

void PhysicsComponent::set_position(vcl::vec3 position) {
    this->position = position;
    this->nextPosition = position;
}

void PhysicsComponent::set_speed(vcl::vec3 speed) {
    velocity = speed;
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

    if (player != nullptr)
        player->nextForce = vcl::vec3(0.0f, 0.0f, 0.0f);
}

void PhysicsComponent::singleUpdate() {
    int n = objects.size();

    // First compute the physics of the camera
    if (player != nullptr) {
        player->accel = vcl::vec3(0.0f, 0.0f, 0.0f);
        for (int j = 0; j < n; j++) {
            PhysicsComponent* obj = objects[j];
            float sqrDist = obj->position.x * obj->position.x;
            sqrDist += obj->position.y * obj->position.y;
            sqrDist += obj->position.z * obj->position.z;
            player->accel -= obj->mass / sqrDist * vcl::normalize(-obj->position);
        }
        player->accel *= G;
        player->accel += player->nextForce / player->mass;
        player->currentSpeed += player->accel * fixedDeltaTime;
    }

    // Update current position
    for (int i = 0; i < n; i++) {
        objects[i]->position = objects[i]->nextPosition;
    }

    if (player != nullptr)
        player->clamp_to_planets();

    // Compute the physics of the other planets
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

        if (player != nullptr) {
            accel -= player->accel;
            current->velocity += accel * fixedDeltaTime;
            current->nextPosition = current->position + (current->velocity - player->additionalSpeed) * fixedDeltaTime;
        }
        else {
            current->velocity += accel * fixedDeltaTime;
            current->nextPosition = current->position + current->velocity * fixedDeltaTime;
        }
    }    
}