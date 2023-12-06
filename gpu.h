#pragma once

#include <vulkan/vulkan.h>
#include <optional>

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	QueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface);

	bool isComplete();
};

class GPU {
public:
	VkPhysicalDevice physical_gpu;
	VkDevice logical_gpu;

	GPU(VkPhysicalDevice physical, VkDevice logical);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	void allocateMemory(VkDeviceSize size, uint32_t mem_type_index, VkDeviceMemory& memory);
};