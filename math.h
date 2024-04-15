#pragma once
#include <glm/glm.hpp>

glm::vec3 normalize(glm::vec3 vector);
glm::mat4 matmul(glm::mat4 A, glm::mat4 B);
glm::vec3 mul(glm::mat4 M, glm::vec3 v);
glm::vec3 cross(glm::vec3 x, glm::vec3 y);
float dot(glm::vec4 x, glm::vec4 y);
glm::mat4 transpose(glm::mat4 matrix);

template<typename T>
void argsort(std::vector<T>& array, std::vector<int>& out) {
	/*
	Return the indices order that sorts the array in the decending order
	*/

	// instantiate the index array
	out.resize(array.size());
	for (int i = 0; i < out.size(); i++) out[i] = i;

	// bubble sort algorithm
	for (int i = out.size() - 1; i > 0; i--) {
		bool swapped = false;
		for (int j = 0; j < i; j++) {
			if (array[out[j]] < array[out[j + 1]]) {
				int temp = out[j];
				out[j] = out[j + 1];
				out[j + 1] = temp;
				swapped = true;
			}
		}
		if (!swapped) break;
	}
}