#pragma once

#include <vector>
#include <fstream>

#include "vertex.h"
#include "light.h"
#include "camera.h"
#include "buffer.h"

const int MAX_FRAMES_IN_FLIGHT = 2;

class MeshBase {
	/*
	The base mesh class. Every mesh have indices, an initial transformation,
	an index offset, a vertex offset, a diffuse texture index
	*/
public:
	std::vector<uint32_t> indices;
	glm::mat4 init_transform;
	int index_offset;
	int vertex_offset;
	int texture_index;
	std::string debug_node_name;
};

class Mesh : public MeshBase {
public:
	std::vector<Vertex> vertices;

	// serialize this mesh to the file
	void serialize(std::ofstream& file);

	// deserialize the mesh from the file
	void deserialize(std::ifstream& file);
};

class MeshWithNormalMap : public MeshBase {
public:
	std::vector<VertexWithTangent> vertices;
	int normal_map_index;

	// serialize this mesh to the file
	void serialize(std::ofstream& file);

	void calculate_tangent_vectors();
};

struct Texture {
	std::string file_path;

	// constructor
	Texture();
	Texture(std::string path);

	// serialize this texture to the file
	void serialize(std::ofstream& file);

	// deserialize this texture from the file
	void deserialize(std::ifstream& file);
};

struct NormalMap {
	std::string file_path;
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
	std::vector<MeshWithNormalMap> meshes_with_normal_map;
	std::vector<Texture> textures;
	std::vector<NormalMap> normal_maps;
	std::vector<std::string> debug_node_names;
	int debug_index;
	bool debug_press_n;
	bool debug_press_b;
	bool debug_press_t;
	bool debug_mode;
	bool enable_normal_map;
	light lights;
	Camera camera;
	Buffer* vertex_buffer;
	Buffer* index_buffer;
	Buffer* uniform_buffer;
	void* uniformBuffersMapped;

	~Scene();
	
	// Get the number of vertices in the scene
	int get_num_vertices();

	// Get the number of vertices with tangent vector
	int get_num_vertices_with_tangent();

	// Get the total number of indices in the scene
	int get_num_indices();

	void createVertexBuffer(GPU* gpu);

	void createIndexBuffer(GPU* gpu);

	void createUniformBuffer(GPU* gpu);
};