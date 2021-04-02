#pragma once

#include "vcl/vcl.hpp"
#include "planet.hpp"


class PostProcessing {
private:

	static GLuint quadVao;
	static GLuint quadVbo;
	static GLuint initialized;


	GLuint framebuffer;
	GLuint textureColorbuffer;
	GLuint depthbuffer;
	GLuint shader;

public:
	static GLuint defaultShader;

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

