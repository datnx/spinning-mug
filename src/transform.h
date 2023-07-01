#pragma once
#include <glm/glm.hpp>

glm::mat4 change_basis(glm::mat4 init, glm::vec3 new_x, glm::vec3 new_y, glm::vec3 new_z);
glm::mat4 scale(glm::mat4 init, float scale_x, float scale_y, float scale_z);
glm::mat4 translate(glm::mat4 init, glm::vec3 t);
glm::mat4 rotate(glm::mat4 m, float angle, glm::vec3 axis);
glm::mat4 lookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up);
glm::mat4 perspective(float fovy, float aspect, float near, float far);