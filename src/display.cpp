#include "vcl/vcl.hpp"
#include "display.hpp"
#include "vegetation.hpp"
#include "planet.hpp"
#include <algorithm>


using namespace vcl;

void buildFrustrsums(scene_environment& scene, unsigned int width, unsigned int height, float interPlane) {
	float farPlane = 10000.0f;
	float nearPlane = 0.1f;
	float const aspect = width / static_cast<float>(height);
	scene.farProjection = projection_perspective(pi / 3, aspect, interPlane, farPlane);
	scene.nearProjection = projection_perspective(pi / 3, aspect, nearPlane, interPlane);
	Planet::interPlane = interPlane;
	Planet::nearPlane = nearPlane;
	Planet::farPlane = farPlane;
}

struct SortingPlanet {
	Planet* pointer;
	float distance;

	SortingPlanet(Planet* planet, float distance) {
		pointer = planet;
		this->distance = distance;
	}
};

bool operator<(const SortingPlanet& first, const SortingPlanet& second)
{
	return first.distance < second.distance;
}

static void drawPlant(const scene_environment& scene, hierarchy_mesh_drawable& plant, Planet& planet, vcl::vec3 position, float alpha, float scale) {
	plant["troncon 0"].transform.translate = planet.getPlanetRadiusAt(position) * 0.999f;
	if (vcl::norm(plant["troncon 0"].transform.translate + planet.getPosition()) > 15.0f) {
		return;
	}
	vcl::vec3 up = position;
	vcl::vec3 ortho = vec3(1.0f, 0.0f, 0.0f);
	vcl::vec3 tempCross = cross(up, ortho);
	if (norm(tempCross) < 0.01f)
		return;
	vcl::vec3 right = vcl::normalize(tempCross);
	vcl::vec3 back = vcl::cross(right, up);
	vcl::mat3 rotationMatrix({ back, right, up });
	plant["troncon 0"].transform.rotate = vcl::rotation(up, alpha) * vcl::rotation(transpose(rotationMatrix));
	plant["troncon 0"].transform.scale = scale;
	plant["troncon 0"].transform = planet.visual.transform * plant["troncon 0"].transform;
	plant.update_local_to_global_coordinates();
	draw(plant, scene);
}


void display_scene(scene_environment& scene, float time, float width, float height) {
	plantAnimation(scene.plant, time * 0.5f);

	scene.light = scene.planets[0].getPosition();

	// Find the planet close to the player if it exists
	// Create an adaptative frustrum for the multipass render
	std::vector<SortingPlanet> farPlanets;
	std::vector<int> nearPlanetIndices;
	float separatingPlane = midDistance;
	for (int i = 0; i < scene.planets.size(); i++) {
		//float dist = vcl::norm(scene.planets[i].getPosition() - scene.camera.position());
		float dist = vcl::norm(scene.planets[i].getPosition() - scene.camera.position());
		float changeDist = scene.planets[i].radius * (scene.planets[i].hasAtmosphere ? std::max(scene.planets[i].atmosphereHeight, 1.3f) : 1.3f);
		if (dist > separatingPlane + changeDist || !CAMERA_TYPE) {
			if (dot(scene.camera.front(), scene.planets[i].getPosition()) > 0 || !CAMERA_TYPE)
				farPlanets.push_back(SortingPlanet(&scene.planets[i], dist));
		}
		else {
			nearPlanetIndices.push_back(i);
			if (dist > separatingPlane - changeDist)
				separatingPlane += changeDist;
		}
	}

	if (nearPlanetIndices.empty())
		buildFrustrsums(scene, width, height, midDistance);
	else {
		buildFrustrsums(scene, width, height, separatingPlane);
	}


	std::sort(farPlanets.begin(), farPlanets.end());

	// Render all the distant planets on the screen
	scene.projection = scene.farProjection;
	Planet::startPlanetRendering();
	for (int i = 0; i < farPlanets.size(); i++) {
		farPlanets[i].pointer->renderPlanet(scene, farPlanets[i].distance > 200.0f);
	}

	if (farPlanets.size() > 0 || farPlanets.empty())
		scene.skybox.render(scene);

	Planet::startWaterRendering(scene, false);
	for (int i = farPlanets.size() - 1; i >= 1; i--) {
		Planet::switchIntermediateTexture();
		farPlanets[i].pointer->renderWater(scene);
	}

	if (nearPlanetIndices.empty()) {
		Planet::renderFinalPlanet();
		if (farPlanets.size() > 0)
			farPlanets[0].pointer->renderWater(scene);
		else
			scene.skybox.render(scene);
	}
	else {
		if (farPlanets.size() > 0) {
			Planet::switchIntermediateTexture();
			farPlanets[0].pointer->renderWater(scene);
		}
		// We now need to render the near planet with the texture as the background.
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		scene.projection = scene.nearProjection;
		for (int i = 0; i < nearPlanetIndices.size(); i++) {
			scene.planets[nearPlanetIndices[i]].renderPlanet(scene);
			if (nearPlanetIndices[i] == 2) {
				for (int j = 0; j < scene.plantInfos[0].size(); j++) {
					vec3 pos = vcl::vec3(scene.plantInfos[2][j], scene.plantInfos[3][j], scene.plantInfos[4][j]);
					drawPlant(scene, scene.plant, scene.planets[2], pos, scene.plantInfos[5][j], scene.plantInfos[9][j] / 4);
				}
			}
		}

		Planet::startWaterRendering(scene, true);
		for (int i = 0; i < nearPlanetIndices.size() - 1; i++) {
			Planet::switchIntermediateTexture();
			scene.planets[nearPlanetIndices[i]].renderWater(scene);
		}
		Planet::renderFinalPlanet();
		scene.planets[nearPlanetIndices.back()].renderWater(scene);
	}
}

void display_interface(scene_environment& scene, int* planet_index) {
	ImGui::SliderInt("Planet index", planet_index, 0, scene.planets.size() - 1);
	scene.planets[*planet_index].displayInterface();
}

static void opengl_uniform(GLuint shader, scene_environment const& current_scene) {
	opengl_uniform(shader, "projection", current_scene.projection, false);
	opengl_uniform(shader, "view", current_scene.camera.matrix_view(), false);
	opengl_uniform(shader, "light", current_scene.light, false);
}
