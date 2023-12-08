#include "scene.h"

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

void Scene::createVertexBuffer(GPU* gpu) {
    VkDeviceSize bufferSize = sizeof(Vertex) * get_num_vertices();

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
    vkUnmapMemory(gpu->logical_gpu, staging_buffer.memory);

    vertex_buffer = Buffer(gpu, bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    gpu->copyBuffer(staging_buffer.buffer, vertex_buffer.buffer, bufferSize);

    vkDestroyBuffer(gpu->logical_gpu, staging_buffer.buffer, nullptr);
    vkFreeMemory(gpu->logical_gpu, staging_buffer.memory, nullptr);
}

void Scene::createIndexBuffer(GPU* gpu) {
    VkDeviceSize bufferSize = sizeof(uint16_t) * get_num_indices();

    Buffer staging_buffer(gpu, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(gpu->logical_gpu, staging_buffer.memory, 0, bufferSize, 0, &data);
    size_t offset = 0;
    for (int i = 0; i < meshes.size(); i++) {
        size_t indices_size = sizeof(uint16_t) * meshes[i].indices.size();
        memcpy((char*)data + offset, meshes[i].indices.data(), indices_size);
        offset += indices_size;
    }
    vkUnmapMemory(gpu->logical_gpu, staging_buffer.memory);

    index_buffer = Buffer(gpu, bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    gpu->copyBuffer(staging_buffer.buffer, index_buffer.buffer, bufferSize);

    vkDestroyBuffer(gpu->logical_gpu, staging_buffer.buffer, nullptr);
    vkFreeMemory(gpu->logical_gpu, staging_buffer.memory, nullptr);
}