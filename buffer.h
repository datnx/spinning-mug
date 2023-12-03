#pragma once

#include <vulkan/vulkan.h>

class Buffer {
public:
	
	// the device that store the buffer
	VkDevice device;

	// the buffer handle
	VkBuffer buffer;

	// the memory that this buffer is at
	VkDeviceMemory memory;

	// destructor
	~Buffer();
};