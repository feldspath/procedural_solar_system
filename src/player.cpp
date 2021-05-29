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

void Player::update_position(vcl::int3 direction) {
	vcl::vec3 dir = direction.x * camera->front() + direction.y * camera->right() + direction.z * camera->up();
	float dir_norm = vcl::norm(dir);

    // Movement
    if (!onGround) {
        // If we're in space, use the jetpack
        if (jetpackOn && dir_norm > 0.1f)
            nextForce = thrustForce * dir / dir_norm;
    }
    else {
        // If we're on a planet, we walk
        if (dir_norm > 0.1f)
            additionalSpeed = dir / dir_norm * walkSpeed;
        else
            additionalSpeed = vcl::vec3(0.0f, 0.0f, 0.0f);
        
        if (jetpackOn && dir_norm > 0.1f)
            nextForce = thrustForce * dir / dir_norm;
    }

    // Orientation
    if (currentPlanet != -1) {
        // We update the camera orientation
        vcl::vec3 newRight = vcl::normalize(vcl::cross(camera->front(), -planets->at(currentPlanet).getPosition()));
        vcl::vec3 newUp = vcl::normalize(vcl::cross(newRight, camera->front()));
        vcl::mat3 newMatrix({ newRight, newUp, -camera->front() });
        camera->orientation_camera = vcl::rotation(vcl::transpose(newMatrix));
    }
    camera->position_camera = vcl::vec3(0.0f, 0.0f, 0.0f);
}

void Player::clamp_to_planets() {
    for (int i = 0; i < planets->size(); i++) {
        vcl::vec3 planetPos = planets->at(i).getPosition();
        vcl::vec3 playerPosToPlanet = -planetPos;

        vcl::mat4 planetModel = planets->at(i).visual.transform.matrix();
        vcl::vec4 posOnSphere = vcl::inverse(planetModel) * vcl::vec4(playerPosToPlanet.x, playerPosToPlanet.y, playerPosToPlanet.z, 0.0f);
        vcl::vec3 posOnSphere3 = vcl::vec3(posOnSphere.x, posOnSphere.y, posOnSphere.z);

        float playerHeight = vcl::norm(posOnSphere3);
        float heightAtPlayer = vcl::norm(planets->at(i).getPlanetRadiusAt(posOnSphere3 / playerHeight));

        // if player is too close from the planet
        if (playerHeight < heightAtPlayer + height) {
            // set the player height to the correct one
            vcl::vec3 newPos = playerPosToPlanet / playerHeight * (heightAtPlayer + height) + planetPos;
            for (int i = 0; i < PhysicsComponent::objects.size(); i++) {
                PhysicsComponent::objects[i]->set_position(PhysicsComponent::objects[i]->get_position() - newPos);
            }

            // reset its speed relative to the planet
            vcl::vec3 newSpeed = planets->at(i).getSpeed() + planets->at(i).rotateSpeed * vcl::cross(vcl::vec3(0.0f, 0.0f, 1.0f), newPos - planetPos) + currentSpeed + additionalSpeed;
            for (int i = 0; i < PhysicsComponent::objects.size(); i++)
                PhysicsComponent::objects[i]->set_speed(PhysicsComponent::objects[i]->get_speed() - newSpeed + currentSpeed + additionalSpeed);

            currentSpeed = vcl::vec3(newSpeed.x, newSpeed.y, newSpeed.z);

            currentPlanet = i;
            onGround = true;
            break;
        }
        else if (i == currentPlanet) {
            if (playerHeight > planets->at(i).radius * (planets->at(i).hasAtmosphere ? planets->at(i).atmosphereHeight : 1.5f))
                currentPlanet = -1;
            else
                onGround = false;
        }
    }
}

void Player::toggleJetpack() {
    jetpackOn = !jetpackOn;
    std::cout << "Jetpack is " << jetpackOn << std::endl;
}