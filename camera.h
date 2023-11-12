#pragma once

#include <glm/vec3.hpp>
#include <GLFW/glfw3.h>

struct Camera {
	/*
	The camera to move around
	*/

	// location and orientation
	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
	float yaw;
	float pitch;

	// a trick to avoid large camera swing in the beginning
	bool firstMouse;

	// a reference point to calculate camera moving speed
	float last_frame;

	// cursor coordinates
	float lastX, lastY;

	// constructors
	Camera(glm::vec3 pos, glm::vec3 front, glm::vec3 up);
	Camera();

	// mouse callback
	void mouse_callback(double xpos, double ypos);

	void awsd_movement(GLFWwindow* window);
};