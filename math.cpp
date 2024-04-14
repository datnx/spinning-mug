#include <glm/gtc/matrix_access.hpp>
#include <cmath>
#include "math.h"

glm::vec3 normalize(glm::vec3 vector) {
	float magnitude = sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
	return glm::vec3(vector.x / magnitude, vector.y / magnitude, vector.z / magnitude);
}

glm::mat4 matmul(glm::mat4 A, glm::mat4 B) {
	glm::mat4 mul;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			mul[j][i] = dot(glm::row(A, i), glm::column(B, j));
		}
	}
	return mul;
}

glm::vec3 mul(glm::mat4 M, glm::vec3 v) {
	float x, y, z, w;
	x = dot(glm::row(M, 0), glm::vec4(v, 1.0f));
	y = dot(glm::row(M, 1), glm::vec4(v, 1.0f));
	z = dot(glm::row(M, 2), glm::vec4(v, 1.0f));
	w = dot(glm::row(M, 3), glm::vec4(v, 1.0f));
	return glm::vec3(x / w, y / w, z / w);
}

float dot(glm::vec4 x, glm::vec4 y) {
	return x[0] * y[0] + x[1] * y[1] + x[2] * y[2] + x[3] * y[3];
}

glm::vec3 cross(glm::vec3 x, glm::vec3 y) {
	return glm::vec3(
		x[1] * y[2] - x[2] * y[1],
		x[2] * y[0] - x[0] * y[2],
		x[0] * y[1] - x[1] * y[0]
	);
}

glm::mat4 transpose(glm::mat4 matrix) {
	return glm::mat4(
		glm::row(matrix, 0),
		glm::row(matrix, 1),
		glm::row(matrix, 2),
		glm::row(matrix, 3)
	);
}