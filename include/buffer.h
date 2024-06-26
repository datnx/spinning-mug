#pragma once

#include <vulkan/vulkan.h>

#include "gpu.h"

class Buffer {
public:
	
	// the GPU that store the buffer
	GPU* gpu;

	// the buffer handle
	VkBuffer buffer;

	// the memory that this buffer is at
	VkDeviceMemory memory;

	// constructor
	Buffer();
	Buffer(GPU* gpu_, VkDeviceSize size, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties);

	// destructor
	~Buffer();
};