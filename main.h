#pragma once
#include "glm/glm.hpp"


glm::vec3 camerDirection = glm::vec3(0.0f, 1.0f, 0.0f); // up
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f); // position
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, -1.0f); //front

double yaw = -90.0f;
double pitch = 0.0f;

double xPos;
double yPos;
double currentX;
double currentY;
bool firstEntered = true;

bool isForward = false;
bool isBackward = false;
bool isUp = false;
bool isDown = false;
bool isLeft = false;
bool isRight = false;