#pragma once
#include <glm/glm.hpp>

glm::vec3 normalize(glm::vec3 vector);
glm::mat4 matmul(glm::mat4 A, glm::mat4 B);
glm::vec3 cross(glm::vec3 x, glm::vec3 y);
float dot(glm::vec4 x, glm::vec4 y);
glm::mat4 transpose(glm::mat4 matrix);