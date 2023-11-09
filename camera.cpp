#include "camera.h"

Camera::Camera(glm::vec3 pos, glm::vec3 front, glm::vec3 up) {
	cameraPos = pos;
	cameraFront = front;
	cameraUp = up;
}

Camera::Camera() {
	cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
}