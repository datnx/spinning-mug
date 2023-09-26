#pragma once

#include <vulkan/vulkan.h>

class MemoryAllocator {
private:
	VkPhysicalDevice physicalDevice;
	VkDevice device;
public:
	MemoryAllocator(VkPhysicalDevice pdevice, VkDevice ldevice);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void allocateMemory(VkDeviceSize size, uint32_t mem_type_index, VkDeviceMemory& memory);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
};