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
void cursorEnterCallback(GLFWwindow* window, double x, double y);
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
	window = glfwCreateWindow(1024, 768, "Virtual Musuem", NULL, NULL);
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
	glfwSetCursorPosCallback(window, cursorEnterCallback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#pragma endregion

#pragma region Mesh Loading

	ObjData hylian;
	LoadObjFile(&hylian, "Hylian_Shield.obj");
	//LoadObjFile(&hylian, "Hylian_Shield.obj");
	GLfloat hylianOffset[] = { 0.0f, 0.0f, 0.0f };
	LoadObjToMemory(&hylian, 1.0f, hylianOffset);

	ObjData pedestal;
	LoadObjFile(&pedestal, "Pedestal.obj");
	GLfloat pedestalOffset[] = { 0.0f, 0.0f, 0.0f };
	LoadObjToMemory(&pedestal, 1.0f, pedestalOffset);


	std::vector <std::string>faces{
		"right.png",
		"left.png",
		"top.png",
		"bottom.png",
		"front.png",
		"back.png"
	};
	SkyboxData skybox = LoadSkybox("Assets/skybox", faces);

#pragma endregion

#pragma region Shader Loading

	GLuint skyboxShaderProgram = LoadShaders("Shaders/skybox_vertex.shader", "Shaders/skybox_fragment.shader");

	GLuint shaderProgram = LoadShaders("Shaders/phong_vertex.shader", "Shaders/earth_night_fragment.shader");
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
	GLuint lightDirLoc = glGetUniformLocation(shaderProgram, "u_light_dir");
	GLuint diffuseTexLoc = glGetUniformLocation(shaderProgram, "texture_diffuse");
	GLuint nightTexLoc = glGetUniformLocation(shaderProgram, "night_diffuse");

	glUniform1i(diffuseTexLoc, 0);
	glUniform1i(nightTexLoc, 1);

	glUniform3f(lightPosLoc, 0.0f, 0.0f, 0.0f);
	glUniform3f(lightDirLoc, 0.0f, 0.0f, -1.0f);

#pragma endregion

	// set bg color to green
	glClearColor(0.4f, 0.4f, 0.0f, 0.0f);

	// var for rotations
	float xFactor = 0.0f;
	float xSpeed = 10.0f;
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
		projection = glm::perspective(glm::radians(90.0f), ratio, 0.1f, 100.0f);
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
		// Set projection matrix in shader


#pragma endregion

#pragma region Bool Handler
		
		if (isForward) {
			cameraPos += cameraTarget * deltaTime * xSpeed;
		}
		if (isRight) {
			cameraPos += glm::normalize(glm::cross(cameraTarget, camerDirection)) * deltaTime * xSpeed;
		}
		if (isLeft) {
			cameraPos -= glm::normalize(glm::cross(cameraTarget, camerDirection)) * deltaTime * xSpeed;
		}
		if (isBackward) {
			cameraPos -= cameraTarget * deltaTime * xSpeed;
		}
		/*glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));*/
#pragma endregion

#pragma region View
		glm::mat4 view;
		view = glm::lookAt(
			/*glm::vec3(0.0f, -10.0f, 0.0f),*/
			cameraPos,
			cameraPos + cameraTarget,
			camerDirection
		);
		glUniform3f(cameraPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
#pragma endregion

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//toggle to render wit GL_FILL or GL_LINE
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

#pragma region Draw
		DrawSkybox(skybox, skyboxShaderProgram, view, projection);

		// Hylian Shield
		glBindVertexArray(hylian.vaoId);
		glUseProgram(shaderProgram);

		trans = glm::mat4(1.0f);
		trans = glm::rotate(trans, glm::radians(90.f), glm::vec3(0.0f, 0.1f, 0.0f));
		trans = glm::translate(trans, glm::vec3(50.0f, -10.0f, -50.0f));
		trans = glm::scale(trans, glm::vec3(1.0f, 1.0f, 1.0f));

		glm::mat4 normalTrans = glm::transpose(glm::inverse(trans));
		glUniformMatrix4fv(normalTransformLoc, 1, GL_FALSE, glm::value_ptr(normalTrans));
		glUniform1i(isLit, false);
		//send to shader
		glUniformMatrix4fv(modelTransformLoc, 1, GL_FALSE, glm::value_ptr(trans));

		glActiveTexture(GL_TEXTURE0);
		GLuint hylianTexture = hylian.textures[hylian.materials[0].diffuse_texname];
		glBindTexture(GL_TEXTURE_2D, hylianTexture);

		//tentative for texturing
		glActiveTexture(GL_TEXTURE1);
		GLuint nightTexture = hylian.textures[hylian.materials[1].diffuse_texname];
		glBindTexture(GL_TEXTURE_2D, nightTexture);

		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUniformMatrix4fv(modelTransformLoc, 1, GL_FALSE, glm::value_ptr(trans));
		glDrawElements(GL_TRIANGLES, hylian.numFaces, GL_UNSIGNED_INT, (void*)0);
		glBindTexture(GL_TEXTURE_2D, 0);

		// PEDESTAL
		glBindVertexArray(pedestal.vaoId);
		glUseProgram(shaderProgram);
		glDisable(GL_BLEND);
		trans = glm::mat4(1.0f);
		trans = glm::rotate(trans, glm::radians(90.f), glm::vec3(0.0f, 0.1f, 0.0f));
		trans = glm::translate(trans, glm::vec3(35.0f, -50.0f, -45.0f));
		trans = glm::scale(trans, glm::vec3(0.5f, 0.5f, 0.5f));

		normalTrans = glm::transpose(glm::inverse(trans));
		glUniformMatrix4fv(normalTransformLoc, 1, GL_FALSE, glm::value_ptr(normalTrans));
		glUniform1i(isLit, false);
		//send to shader
		glUniformMatrix4fv(modelTransformLoc, 1, GL_FALSE, glm::value_ptr(trans));

		glActiveTexture(GL_TEXTURE0);
		GLuint pedestalTexture = pedestal.textures[pedestal.materials[0].diffuse_texname];
		glBindTexture(GL_TEXTURE_2D, pedestalTexture);

		glDrawElements(GL_TRIANGLES, pedestal.numFaces, GL_UNSIGNED_INT, (void*)0);
		glBindTexture(GL_TEXTURE_2D, 0);

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
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		if (key == GLFW_KEY_W) {
			isForward = true;
		}
		if (key == GLFW_KEY_A) {
			isLeft = true;
		}
		if (key == GLFW_KEY_S) {
			isBackward = true;
		}
		if (key == GLFW_KEY_D) {
			isRight = true;
		}
		// enables cursor for easier closing
		if (key == GLFW_KEY_ESCAPE) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			//stops character from moving
			isForward = false;
			isLeft = false;
			isRight = false;
			isBackward = false;
		}
	}
	else if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_W) {
			isForward = false;
		}
		if (key == GLFW_KEY_A) {
			isLeft = false;
		}
		if (key == GLFW_KEY_S) {
			isBackward = false;
		}
		if (key == GLFW_KEY_D) {
			isRight = false;
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
	}
}

void cursorEnterCallback(GLFWwindow* window, double x, double y) {
	if (firstEntered) {
		xPos = x;
		yPos = y;
		firstEntered = false;
	}

	float xOffset = (x - xPos);
	float yOffset = (yPos - y);

	float sensitivity = 0.1f;
	xOffset *= sensitivity;
	yOffset *= sensitivity;

	xPos = x;
	yPos = y;

	yaw += xOffset;
	pitch += yOffset;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraTarget = glm::normalize(front);
}