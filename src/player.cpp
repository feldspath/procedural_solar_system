#include "player.hpp"
#include "planet.hpp"
#include "vcl/vcl.hpp"
#include "physics.hpp"

void Player::bind_camera(camera_fps* camera) {
	this->camera = camera;
}

void Player::bind_planets(std::vector<Planet>* planets) {
	this->planets = planets;
}

void Player::init_physics() {
	physics = PhysicsComponent::generatePhysicsComponent(mass, camera->position_camera);
}

void Player::update_position(vcl::int3 direction) {
	vcl::vec3 dir = direction.x * camera->front() + direction.y * camera->right() + direction.z * camera->up();
	float dir_norm = vcl::norm(dir);

    // Movement
    if (!onGround) {
        // If we're in space, use the jetpack
        if (dir_norm > 0.1f)
            physics->add_force(thrustForce * dir / dir_norm);
    }
    else {
        // If we're on a planet, we walk
        if (dir_norm > 0.1f)
            physics->additionalSpeed = dir / dir_norm * walkSpeed;
        else
            physics->additionalSpeed = vcl::vec3(0.0f, 0.0f, 0.0f);
        
        // We can still thrust up
        if (direction.z == 1)
            physics->add_force(thrustForce * camera->up());
    }

    // Orientation
    if (currentPlanet != -1) {
        // We update the camera orientation
        vcl::vec3 newRight = vcl::normalize(vcl::cross(camera->front(), physics->get_position() - planets->at(currentPlanet).getPosition()));
        vcl::vec3 newUp = vcl::normalize(vcl::cross(newRight, camera->front()));
        vcl::mat3 newMatrix({ newRight, newUp, -camera->front() });
        camera->orientation_camera = vcl::rotation(vcl::transpose(newMatrix));
    }
    camera->position_camera = physics->get_position();
}

void Player::clamp_to_planets() {
    for (int i = 0; i < planets->size(); i++) {
        vcl::vec3 camPos = physics->get_position();
        vcl::vec3 planetPos = planets->at(i).getPosition();
        vcl::vec3 playerPosToPlanet = camPos - planetPos;

        vcl::mat4 planetModel = planets->at(i).visual.transform.matrix();
        vcl::vec4 posOnSphere = vcl::inverse(planetModel) * vcl::vec4(playerPosToPlanet.x, playerPosToPlanet.y, playerPosToPlanet.z, 0.0f);
        vcl::vec3 posOnSphere3 = vcl::vec3(posOnSphere.x, posOnSphere.y, posOnSphere.z);

        float playerHeight = vcl::norm(posOnSphere3);
        float heightAtPlayer = vcl::norm(planets->at(i).getPlanetRadiusAt(posOnSphere3 / playerHeight));

        // if player is too close from the planet
        if (playerHeight < heightAtPlayer + height) {
            // set the player height to the correct one
            vcl::vec3 newPos = playerPosToPlanet / playerHeight * (heightAtPlayer + height) + planetPos;
            physics->set_position(vcl::vec3(newPos.x, newPos.y, newPos.z));

            // reset its speed relative to the planet
            vcl::vec3 newSpeed = planets->at(i).getSpeed() + planets->at(i).rotateSpeed * vcl::cross(vcl::vec3(0.0f, 0.0f, 1.0f), newPos - planetPos);
            physics->set_speed(vcl::vec3(newSpeed.x, newSpeed.y, newSpeed.z));

            currentPlanet = i;
            onGround = true;
            break;
        }
        else if (i == currentPlanet) {
            if (playerHeight > 1.3f * planets->at(i).radius)
                currentPlanet = -1;
            else
                onGround = false;
        }
    }
}