#include <stdexcept>

#include "buffer.h"

Buffer::Buffer() {
	gpu = nullptr;
	buffer = VK_NULL_HANDLE;
	memory = VK_NULL_HANDLE;
}

Buffer::Buffer(GPU* gpu_, VkDeviceSize size, VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties)
{

	// device
	gpu = gpu_;
	
	// create info
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	// create
	if (vkCreateBuffer(gpu->logical_gpu, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	// get memory requirement
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(gpu->logical_gpu, buffer, &memRequirements);

	// find memory type
	uint32_t mem_type = gpu->findMemoryType(memRequirements.memoryTypeBits, properties);

	// allocate the memory and bind it to the buffer
	gpu->allocateMemory(memRequirements.size, mem_type, memory);
	vkBindBufferMemory(gpu->logical_gpu, buffer, memory, 0);
}

Buffer::~Buffer() {
	/*
	Destroy the buffer and free the memory
	*/
	vkDestroyBuffer(gpu->logical_gpu, buffer, nullptr);
	vkFreeMemory(gpu->logical_gpu, memory, nullptr);
}