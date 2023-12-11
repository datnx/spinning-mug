#pragma once
#include <glm/glm.hpp>
#include <assimp/types.h>

glm::vec3 normalize(glm::vec3 vector);
glm::mat4 matmul(glm::mat4 A, glm::mat4 B);
glm::vec3 mul(aiMatrix4x4 M, aiVector3D v);
glm::vec3 mul(aiMatrix3x3 M, aiVector3D v);
glm::vec3 mul(glm::mat4 M, glm::vec3 v);
glm::vec3 cross(glm::vec3 x, glm::vec3 y);
float dot(glm::vec4 x, glm::vec4 y);
glm::mat4 transpose(glm::mat4 matrix);