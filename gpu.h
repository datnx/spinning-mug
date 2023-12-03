#pragma once

#include <vulkan/vulkan.h>

class GPU {
public:
	VkPhysicalDevice physical_gpu;
	VkDevice logical_gpu;

	GPU(VkPhysicalDevice physical, VkDevice logical);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};