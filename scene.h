#pragma once

#include <vector>

#include "vertex.h"
#include "light.h"

class Mesh {
public:
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
	glm::mat4 init_transform;
	std::string material_name;
	std::string debug;
	int index_offset;
	int vertex_offset;
};

class Texture {

};

class Scene {
public:
	std::vector<Mesh> meshes;
	std::vector<Texture> textures;
	light lights;
	
	// Get the total number of vertices in the scene
	int get_num_vertices();

	// Get the total number of indices in the scene
	int get_num_indices();
};