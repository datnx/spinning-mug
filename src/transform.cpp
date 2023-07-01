#include <cmath>
#include "math.h"
#include "transform.h"

glm::mat4 change_basis(glm::mat4 init, glm::vec3 new_x, glm::vec3 new_y, glm::vec3 new_z) {
	glm::mat4 change_basis_matrix(
		glm::vec4(new_x.x, new_y.x, new_z.x, 0.0f),
		glm::vec4(new_x.y, new_y.y, new_z.y, 0.0f),
		glm::vec4(new_x.z, new_y.z, new_z.z, 0.0f),
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
	);
	return matmul(change_basis_matrix, init);
}

glm::mat4 scale(glm::mat4 init, float scale_x, float scale_y, float scale_z) {
	glm::mat4 scale_matrix(
		glm::vec4(scale_x, 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, scale_y, 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, scale_z, 0.0f),
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
	);
	return matmul(scale_matrix, init);
}

glm::mat4 translate(glm::mat4 init, glm::vec3 t) {
	glm:: mat4 translation_matrix(
		glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
		glm::vec4(-t, 1.0f)
	);
	return matmul(translation_matrix, init);
}

glm::mat4 rotate(glm::mat4 init, float angle, glm::vec3 axis) {
	glm::vec3 r = normalize(axis);
	glm::vec3 s;
	if (abs(r[0]) <= abs(r[1]) && abs(r[0]) <= abs(r[2])) {
		s[0] = 0.0f;
		s[1] = -r[2];
		s[2] = r[1];
	} else if (abs(r[1]) <= abs(r[0]) && abs(r[1]) <= abs(r[2])) {
		s[0] = -r[2];
		s[1] = 0.0f;
		s[2] = r[0];
	} else {
		s[0] = -r[1];
		s[1] = r[0];
		s[2] = 0.0f;
	}
	s = normalize(s);
	glm::vec3 t = cross(r, s);
	glm::mat4 M = change_basis(glm::mat4(1.0f), r, s, t);
	glm::mat4 R_x(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, cos(angle), sin(angle), 0.0f,
		0.0f, -sin(angle), cos(angle), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	return matmul(transpose(M), matmul(R_x, matmul(M, init)));
}

glm::mat4 lookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up) {
	glm::vec3 v = normalize(eye - center);
	glm::vec3 r = normalize(cross(up, v));
	glm::vec3 u = cross(v, r);
	return change_basis(translate(glm::mat4(1.0f), eye), r, u, v);
}

glm::mat4 perspective(float fovy, float aspect, float near, float far) {
	float d = cos(fovy / 2.0f) / sin(fovy / 2.0f);
	return glm::mat4(
		glm::vec4(d / aspect, 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, d, 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, far / (near - far), -1.0f),
		glm::vec4(0.0f, 0.0f, far * near / (near - far), 0.0f)
	);
}