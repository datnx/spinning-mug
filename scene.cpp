#include "scene.h"

MeshBase::MeshBase(int num_indices, glm::mat4 i_transform,
	int i_offset, int v_offset, int tex_ind)
{	
	indices.reserve(num_indices);
	init_transform = i_transform;
	index_offset = i_offset;
	vertex_offset = v_offset;
	texture_index = tex_ind;
}

Mesh::Mesh(int num_vertices, int num_indices, glm::mat4 i_transform,
	int i_offset, int v_offset, int tex_ind) :
	MeshBase(num_indices, i_transform, i_offset, v_offset, tex_ind)
{
	vertices.reserve(num_vertices);
}

MeshWithNormalMap::MeshWithNormalMap(int num_vertices, int num_indices,
	glm::mat4 i_transform, int i_offset, int v_offset,
	int tex_ind, int nor_map_ind) :
	MeshBase(num_indices, i_transform, i_offset, v_offset, tex_ind)
{
	vertices.reserve(num_vertices);
	normal_map_index = nor_map_ind;
}

Texture::Texture(std::string path) {
	file_path = path;
}

int Scene::get_num_vertices() {
	int count = 0;
	for (int i = 0; i < meshes.size(); i++) {
		count += meshes[i].vertices.size();
	}
	return count;
}

int Scene::get_num_indices() {
	int count = 0;
	for (int i = 0; i < meshes.size(); i++) {
		count += meshes[i].indices.size();
	}
	return count;
}
