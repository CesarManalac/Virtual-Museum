#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "glm/glm.hpp"
#include "obj_mesh.h";
#include "shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "main.h"
#include "skybox.h"

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);

int main() {
#pragma region Initialization
	//initialize glfw
	if (glfwInit() != GLFW_TRUE) {
		fprintf(stderr, "Failed to initialized! \n");
		return -1;
	}

	// set opengl version to 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create window 
	GLFWwindow* window;
	window = glfwCreateWindow(1024, 768, "Manalac_Cesar", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to load window! \n");
		return -1;
	}
	glfwMakeContextCurrent(window);

	//initialize glew
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}
	glfwSetKeyCallback(window, keyCallBack);
#pragma endregion

#pragma region Shader Loading

	GLuint skyboxShaderProgram = LoadShaders("Shaders/skybox_vertex.shader", "Shaders/skybox_fragment.shader");

	GLuint shaderProgram = LoadShaders("Shaders/phong_vertex.shader", "Shaders/phong_fragment.shader");
	glUseProgram(shaderProgram);

	GLuint colorLoc = glGetUniformLocation(shaderProgram, "u_color");
	glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);

	// initialize MVP
	GLuint modelTransformLoc = glGetUniformLocation(shaderProgram, "u_model");
	GLuint viewLoc = glGetUniformLocation(shaderProgram, "u_view");
	GLuint projectionLoc = glGetUniformLocation(shaderProgram, "u_projection");

	GLuint normalTransformLoc = glGetUniformLocation(shaderProgram, "u_normal");
	GLuint cameraPosLoc = glGetUniformLocation(shaderProgram, "u_camera_pos");
	GLuint ambientColorLoc = glGetUniformLocation(shaderProgram, "u_ambient_color");
	GLuint isLit = glGetUniformLocation(shaderProgram, "u_lit");

	glUniform3f(ambientColorLoc, 0.1f, 0.1f, 0.1f);

	glm::mat4 trans = glm::mat4(1.0f); // identity
	glUniformMatrix4fv(modelTransformLoc, 1, GL_FALSE, glm::value_ptr(trans));

	// define projection matrix
	glm::mat4 projection = glm::mat4(1.0f);

	GLuint lightPosLoc = glGetUniformLocation(shaderProgram, "u_light_pos");
	glUniform3f(lightPosLoc, 0.0f, 0.0f, 0.0f);

#pragma endregion

	// set bg color to green
	glClearColor(0.4f, 0.4f, 0.0f, 0.0f);

	// var for rotations
	float xFactor = 0.0f;
	float xSpeed = 2.0f;
	float currentTime = glfwGetTime();
	float prevTime = 0.0f;
	float deltaTime = 0.0f;

	//depth testing
	glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_ALWAYS); // set the depth test function

	//face culling
	glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK); // set which face to cull
	//glFrontFace(GL_CCW); // set the front face orientation

	while (!glfwWindowShouldClose(window)) {

#pragma region Viewport
		float ratio;
		int width, height;

		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;

		glViewport(0, 0, width, height);
#pragma endregion

#pragma region Projection
		// Orthopgraphic projection but make units same as pixels. origin is lower left of window
		// projection = glm::ortho(0.0f, (GLfloat)width, 0.0f, (GLfloat)height, 0.1f, 10.0f); // when using this scale objects really high at pixel unity size

		// Orthographic with stretching
		//projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 10.0f);

		// Orthographic with corection for stretching, resize window to see difference with previous example
		//projection = glm::ortho(-ratio * 10, ratio * 10, -10.0f, 10.0f, 0.1f, 100.0f);

		// Perspective Projection
		//projection = glm::perspective(glm::radians(90.0f), ratio, 0.1f, 10.0f),

		// Set projection matrix in shader


#pragma endregion

#pragma region Bool Handler
		if (!isForward) {
			projection = glm::ortho(-ratio * 10, ratio * 10, -10.0f, 10.0f, 0.1f, 100.0f);
		}
		else {
			projection = glm::perspective(glm::radians(90.0f), ratio, 0.1f, 100.0f);
		}
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
#pragma endregion

#pragma region View
		glm::mat4 view;
		glm::vec3 cameraPos = glm::vec3(0.0f, -10.0f, 0.0f);
		if (!isForward) {
			view = glm::lookAt(
				/*glm::vec3(0.0f, -10.0f, 0.0f),*/
				cameraPos,
				glm::vec3(0, 0, 0),
				glm::vec3(1.0f, 0.0f, 0.0f));
		}
		else {
			cameraPos = glm::vec3(0.0f, 0.0f, -10.0f);
			view = glm::lookAt(
				/*glm::vec3(0.0f, 0.0f, -10.0f)*/
				cameraPos,
				glm::vec3(0, 0, 0),
				glm::vec3(0.0f, 1.0f, 0.0f));
		}
		glUniform3f(cameraPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
#pragma endregion

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//toggle to render wit GL_FILL or GL_LINE
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

#pragma region Draw
		
		glm::mat4 normalTrans = glm::transpose(glm::inverse(trans));
		glUniformMatrix4fv(normalTransformLoc, 1, GL_FALSE, glm::value_ptr(normalTrans));
		//send to shader
		glUniformMatrix4fv(modelTransformLoc, 1, GL_FALSE, glm::value_ptr(trans));

		currentTime = glfwGetTime();
		deltaTime = currentTime - prevTime;
		prevTime = currentTime;
		xFactor += deltaTime * 20;

		//--- stop drawing here ---
#pragma endregion

		glfwSwapBuffers(window);
		//listen for glfw input events
		glfwPollEvents();
	}
	return 0;
}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS && key == GLFW_KEY_SPACE) {
		if (!isForward) {
			isForward = true;
		}
		else {
			isForward = false;
		}

	}
	else if (action == GLFW_RELEASE) {
		isForward = false;
	}
}