#pragma once

#include <vector>

#include "vertex.h"
#include "light.h"
#include "camera.h"
#include "buffer.h"

class Mesh {
public:
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
	glm::mat4 init_transform;
	int index_offset;
	int vertex_offset;
	int texture_index;
};

struct Texture {
	std::string file_path;
	Texture(std::string path);
};

class Scene {
public:
	std::vector<Mesh> meshes;
	std::vector<Texture> textures;
	light lights;
	Camera camera;
	Buffer vertex_buffer;
	Buffer index_buffer;
	//Buffer uniform_buffer;
	
	// Get the total number of vertices in the scene
	int get_num_vertices();

	// Get the total number of indices in the scene
	int get_num_indices();

	void createVertexBuffer(GPU* gpu);

	void createIndexBuffer(GPU* gpu);
};