#include "post_processing.hpp"

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
	glGenFramebuffers(1, &framebuffer);
	shader = s;
	attachTextureToFramebuffer(width, height);
	opengl_check;
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

void PostProcessing::startRenderToFrameBuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glEnable(GL_DEPTH_TEST);
	opengl_check;
}

void PostProcessing::stopRenderToFrameBuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	opengl_check;
}

void PostProcessing::renderColorbuffer(camera_around_center& camera, const mat4& perspectiveMatrix) {
	assert_vcl(initialized, "Please initialize quad to enable post processing rendering");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.

	// clear all relevant buffers
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shader);
	opengl_uniform(shader, "screenTexture", 0, false);
	opengl_uniform(shader, "depthTexture", 1, false);

	opengl_uniform(shader, "viewMatrix", camera.matrix_view(), false);
	opengl_uniform(shader, "perspectiveInverse", inverse(perspectiveMatrix), false);

	glBindVertexArray(quadVao);
	glActiveTexture(GL_TEXTURE0); // Texture unit 0
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);

	glActiveTexture(GL_TEXTURE1); // Texture unit 1
	glBindTexture(GL_TEXTURE_2D, depthbuffer);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	opengl_check;
}

void PostProcessing::attachTextureToFramebuffer(const unsigned int width, const unsigned int height) {
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glGenTextures(1, &textureColorbuffer);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		opengl_check;
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
		opengl_check;
		// create a renderbuffer object for depth
		glGenTextures(1, &depthbuffer);
		glBindTexture(GL_TEXTURE_2D, depthbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthbuffer, 0);
		opengl_check;


		assert_vcl(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Error : Framebuffer is not complete");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		opengl_check;
	}