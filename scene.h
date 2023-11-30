#pragma once

#include <vector>

#include "vertex.h"
#include "light.h"
#include "camera.h"

class MeshBase {
	/*
	The base mesh class. Every mesh have indices, an initial transformation,
	an index offset, a vertex offset, a diffuse texture index
	*/
public:
	std::vector<uint16_t> indices;
	glm::mat4 init_transform;
	int index_offset;
	int vertex_offset;
	int texture_index;
	MeshBase(int num_indices, glm::mat4 i_transform,
		int i_offset, int v_offset, int tex_ind);
};

class Mesh : public MeshBase {
public:
	std::vector<Vertex> vertices;
	Mesh(int num_vertices, int num_indices, glm::mat4 i_transform,
		int i_offset, int v_offset, int tex_ind);
};

class MeshWithNormalMap : public MeshBase {
public:
	std::vector<VertexWithTangent> vertices;
	int normal_map_index;
	MeshWithNormalMap(int num_vertices, int num_indices, glm::mat4 i_transform,
		int i_offset, int v_offset, int tex_ind, int nor_map_ind);
};

struct Texture {
	std::string file_path;
	Texture(std::string path);
};

class Scene {
public:
	std::vector<Mesh> meshes;
	std::vector<MeshWithNormalMap> meshes_with_normal_map;
	std::vector<Texture> textures;
	light lights;
	Camera camera;
	
	// Get the total number of vertices in the scene
	int get_num_vertices();

	// Get the total number of indices in the scene
	int get_num_indices();
};