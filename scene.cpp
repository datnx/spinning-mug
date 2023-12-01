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

void MeshWithNormalMap::calculate_tangent_vectors() {
	/*
	Calculate the tangent vectors based on the math in
	https://learnopengl.com/Advanced-Lighting/Normal-Mapping
	*/

	// count how many triangles that share each vertex
	std::vector<int> count;
	count.resize(vertices.size());
	for (int i = 0; i < count.size(); i++) count[i] = 0;

	// for each triangle
	for (int i = 0; i < indices.size(); i += 3) {
		
		glm::vec3 edge1 = vertices[indices[i + 1]].pos - vertices[indices[i]].pos;
		glm::vec3 edge2 = vertices[indices[i + 2]].pos - vertices[indices[i + 1]].pos;

		glm::vec2 deltaUV1 = vertices[indices[i + 1]].texCoord - vertices[indices[i]].texCoord;
		glm::vec2 deltaUV2 = vertices[indices[i + 2]].texCoord - vertices[indices[i + 1]].texCoord;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		glm::vec3 tangent;
		tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		
		for (int j = 0; j < 3; j++) {
			vertices[indices[i + j]].tangent += tangent;
			count[indices[i + j]]++;
		}
	}

	// for each vertex
	for (int i = 0; i < vertices.size(); i++) {
		vertices[i].tangent /= count[i];
	}
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
