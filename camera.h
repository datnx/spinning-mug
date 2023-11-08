#pragma once

#include <glm/vec3.hpp>

struct Camera {
	/*
	The camera to move around
	*/
	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
};