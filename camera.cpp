#include <cmath>
#include <glm/trigonometric.hpp>
#include <iostream>

#include "camera.h"
#include "math.h"

Camera::Camera(glm::vec3 pos, glm::vec3 front, glm::vec3 up) {
	cameraPos = pos;
	cameraFront = front;
	cameraUp = up;
	last_frame = 0.0f;
	lastX = 400;
	lastY = 300;
	yaw = -90.0f;
	pitch = 0.0f;
	firstMouse = true;
}

Camera::Camera() {
	cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	last_frame = 0.0f;
	lastX = 400;
	lastY = 300;
	yaw = -90.0f;
	pitch = 0.0f;
	firstMouse = true;
}

void Camera::set_first_mouse() {
	firstMouse = true;
}

void Camera::awsd_movement(GLFWwindow* window) {
	/*
	using awsd to move around
	*/

	float currentFrame = glfwGetTime();
	float deltaTime = currentFrame - last_frame;
	last_frame = currentFrame;
	float cameraSpeed = 2.5f * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= normalize(cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += normalize(cross(cameraFront, cameraUp)) * cameraSpeed;
}

void Camera::mouse_callback(double xpos, double ypos) {
	/*
	this is called each frame to process mouse movements
	*/

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	const float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = normalize(direction);
}