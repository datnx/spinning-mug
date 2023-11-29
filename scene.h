#pragma once

#include <vector>

#include "vertex.h"
#include "light.h"
#include "camera.h"
#include "buffer.h"

const int MAX_FRAMES_IN_FLIGHT = 2;

class Mesh {
public:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	glm::mat4 init_transform;
	int index_offset;
	int vertex_offset;
	int texture_index;
	std::string debug_node_name;
};

class MeshWithNormalMap {
public:
	std::vector<VertexWithTangent> vertices;
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

struct ViewProjectrion {
	glm::mat4 view;
	glm::mat4 proj;
};

struct FragmentUniform {
	light lights;
	alignas(16) glm::vec3 eye;
};

class Scene {
public:
	std::vector<Mesh> meshes;
	std::vector<Texture> textures;
	std::vector<std::string> debug_node_names;
	int debug_index;
	bool debug_press_n;
	bool debug_press_b;
	bool debug_press_t;
	bool debug_mode;
	light lights;
	Camera camera;
	Buffer* vertex_buffer;
	Buffer* index_buffer;
	Buffer* uniform_buffer;
	void* uniformBuffersMapped;

	~Scene();
	
	// Get the total number of vertices in the scene
	int get_num_vertices();

	// Get the total number of indices in the scene
	int get_num_indices();

	void createVertexBuffer(GPU* gpu);

	void createIndexBuffer(GPU* gpu);

	void createUniformBuffer(GPU* gpu);
};