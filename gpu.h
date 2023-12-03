#pragma once

#include <vulkan/vulkan.h>

class GPU {
public:
	VkPhysicalDevice physical_gpu;
	VkDevice logical_gpu;

	GPU(VkPhysicalDevice physical, VkDevice logical);
};