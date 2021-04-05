#pragma once

#include "vcl/vcl.hpp"
#include "planet.hpp"


class PostProcessing {
private:

	static GLuint quadVao;
	static GLuint quadVbo;
	static GLuint initialized;


	GLuint framebuffer = 0;
	GLuint textureColorbuffer = 0;
	GLuint depthbuffer = 0;
	GLuint shader = 0;

public:
	static GLuint defaultShader;

	float waterLevel = 1.0f;
	float depthMultiplier = 6.0f;
	float waterBlendMultiplier = 60.0f;
	vcl::vec3 waterColorDeep = vcl::vec3(0.1f, 0.1f, 0.1f);
	vcl::vec3 waterColorSurface = vcl::vec3(0.3f, 0.5f, 1.0f);

	PostProcessing() {}


	PostProcessing(GLuint s, const unsigned int width, const unsigned int height);

	static void initialiseRenderQuad();
	static void deleteRenderQuad();

	void startRenderToFrameBuffer();
	void stopRenderToFrameBuffer();
	void renderColorbuffer(vcl::camera_around_center& camera, const vcl::mat4& perspectiveMatrix, Planet& planet);

	void buildTextures(const unsigned int width, const unsigned int height);

private:
	void bindTexturesToFramebuffer();
};

