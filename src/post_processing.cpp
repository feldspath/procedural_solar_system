#include "post_processing.hpp"
#include "planet.hpp"

using namespace vcl;

static float quadVertices[] = {
	// positions   // texCoords
	-1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,

	-1.0f,  1.0f,  0.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	 1.0f,  1.0f,  1.0f, 1.0f
};

GLuint PostProcessing::initialized = false;
GLuint PostProcessing::quadVao;
GLuint PostProcessing::quadVbo;
GLuint PostProcessing::defaultShader;

PostProcessing::PostProcessing(GLuint s, const unsigned int width, const unsigned int height) {
	shader = s;
	glGenFramebuffers(1, &framebuffer);
	glGenTextures(1, &depthbuffer);
	glGenTextures(1, &textureColorbuffer);
	buildTextures(width, height);
	bindTexturesToFramebuffer();
}


void PostProcessing::initialiseRenderQuad() {
	assert_vcl(!initialized, "Post processing quad is already initialized");

	glGenVertexArrays(1, &quadVao);
	glGenBuffers(1, &quadVbo);
	glBindVertexArray(quadVao);
	glBindBuffer(GL_ARRAY_BUFFER, quadVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	initialized = true;
	opengl_check;
}

void PostProcessing::deleteRenderQuad() {
	glDeleteVertexArrays(1, &quadVao);
	glDeleteBuffers(1, &quadVbo);
}

void PostProcessing::startRenderToFrameBuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glEnable(GL_DEPTH_TEST);
	opengl_check;
}

void PostProcessing::stopRenderToFrameBuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessing::renderColorbuffer(camera_around_center& camera, const mat4& perspectiveMatrix, Planet& planet) {
	assert_vcl(initialized, "Trying to render a post processing befofre initializing the render quad");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.

	// clear all relevant buffers
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shader);

	// Textures
	opengl_uniform(shader, "screenTexture", 0, false);
	opengl_uniform(shader, "depthTexture", 1, false);

	// Matrices
	opengl_uniform(shader, "viewMatrix", camera.matrix_view(), false);
	opengl_uniform(shader, "perspectiveInverse", inverse(perspectiveMatrix), false);

	// Terrain
	vcl::vec4 center = vec4(planet.visual.transform.translate, 1.0f);
	opengl_uniform(shader, "planetCenter", center);

	// Water
	opengl_uniform(shader, "oceanLevel", waterLevel);
	opengl_uniform(shader, "depthMultiplier", depthMultiplier);
	opengl_uniform(shader, "waterBlendMultiplier", waterBlendMultiplier);

	// Color
	opengl_uniform(shader, "waterColorSurface", waterColorSurface);
	opengl_uniform(shader, "waterColorDeep", waterColorDeep);

	glBindVertexArray(quadVao);
	glActiveTexture(GL_TEXTURE0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);

	glActiveTexture(GL_TEXTURE1); // Texture unit 1
	glBindTexture(GL_TEXTURE_2D, depthbuffer);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	opengl_check;
}

void PostProcessing::buildTextures(const unsigned int width, const unsigned int height) {
	// Texture color buffer
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	opengl_check;

	// Texture depth buffer
	glBindTexture(GL_TEXTURE_2D, depthbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	opengl_check;
}

void PostProcessing::bindTexturesToFramebuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthbuffer, 0);
	opengl_check;

	assert_vcl(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Error : Framebuffer is not complete");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
