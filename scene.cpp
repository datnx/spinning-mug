#include "scene.h"

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

Scene::~Scene() {
    delete vertex_buffer;
    delete index_buffer;
    delete uniform_buffer;
}

int Scene::get_num_vertices() {
	int count = 0;
	for (int i = 0; i < meshes.size(); i++) {
		count += meshes[i].vertices.size();
	}
	return count;
}

int Scene::get_num_vertices_with_tangent() {
	int count = 0;
	for (int i = 0; i < meshes_with_normal_map.size(); i++) {
		count += meshes_with_normal_map[i].vertices.size();
	}
	return count;
}

int Scene::get_num_indices() {
	int count = 0;
	for (int i = 0; i < meshes.size(); i++) {
		count += meshes[i].indices.size();
	}
	for (int i = 0; i < meshes_with_normal_map.size(); i++) {
		count += meshes_with_normal_map[i].indices.size();
	}
	return count;
}

void Scene::createVertexBuffer(GPU* gpu) {
    VkDeviceSize bufferSize = sizeof(Vertex) * get_num_vertices()
		+ sizeof(VertexWithTangent) * get_num_vertices_with_tangent();

    Buffer staging_buffer(gpu, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(gpu->logical_gpu, staging_buffer.memory, 0, bufferSize, 0, &data);
    size_t offset = 0;
    for (int i = 0; i < meshes.size(); i++) {
        size_t vertices_size = sizeof(Vertex) * meshes[i].vertices.size();
        memcpy((char*)data + offset, meshes[i].vertices.data(), vertices_size);
        offset += vertices_size;
    }
	for (int i = 0; i < meshes_with_normal_map.size(); i++) {
		size_t vertices_size = sizeof(VertexWithTangent) * meshes_with_normal_map[i].vertices.size();
		memcpy((char*)data + offset, meshes_with_normal_map[i].vertices.data(), vertices_size);
		offset += vertices_size;
	}
    vkUnmapMemory(gpu->logical_gpu, staging_buffer.memory);

    vertex_buffer = new Buffer(gpu, bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    gpu->copyBuffer(staging_buffer.buffer, vertex_buffer->buffer, bufferSize);
}

void Scene::createIndexBuffer(GPU* gpu) {
    VkDeviceSize bufferSize = sizeof(uint32_t) * get_num_indices();

    Buffer staging_buffer(gpu, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(gpu->logical_gpu, staging_buffer.memory, 0, bufferSize, 0, &data);
    size_t offset = 0;
    for (int i = 0; i < meshes.size(); i++) {
        size_t indices_size = sizeof(uint32_t) * meshes[i].indices.size();
        memcpy((char*)data + offset, meshes[i].indices.data(), indices_size);
        offset += indices_size;
    }
	for (int i = 0; i < meshes_with_normal_map.size(); i++) {
		size_t indices_size = sizeof(uint16_t) * meshes_with_normal_map[i].indices.size();
		memcpy((char*)data + offset, meshes_with_normal_map[i].indices.data(), indices_size);
		offset += indices_size;
	}
    vkUnmapMemory(gpu->logical_gpu, staging_buffer.memory);

    index_buffer = new Buffer(gpu, bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    gpu->copyBuffer(staging_buffer.buffer, index_buffer->buffer, bufferSize);
}

void Scene::createUniformBuffer(GPU* gpu) {
    VkDeviceSize bufferSize = (
        gpu->getAlignSize(sizeof(ViewProjectrion)) +
        gpu->getAlignSize(sizeof(FragmentUniform)) +
        meshes.size() * gpu->getAlignSize(sizeof(glm::mat4))
    ) * MAX_FRAMES_IN_FLIGHT;

    uniform_buffer = new Buffer(gpu, bufferSize,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkMapMemory(gpu->logical_gpu, uniform_buffer->memory, 0, bufferSize, 0, &uniformBuffersMapped);
}