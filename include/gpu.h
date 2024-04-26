#pragma once

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	QueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface);

	bool isComplete();
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;

	SwapChainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface);
};

class GPU {
public:
	VkPhysicalDevice physical_gpu;
	VkDevice logical_gpu;
	uint64_t min_uboOffset;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkCommandPool commandPool;

	GPU();

	GPU(VkInstance vulkan_instance, VkSurfaceKHR surface);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	void allocateMemory(VkDeviceSize size, uint32_t mem_type_index, VkDeviceMemory& memory);

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	VkCommandBuffer beginSingleTimeCommands();

	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	uint64_t getAlignSize(uint64_t size);

	VkShaderModule createShaderModule(const std::vector<char>& code);

	void createImage(uint32_t width, uint32_t height, VkSampleCountFlagBits numSamples, VkFormat format, VkImageUsageFlags usage, VkImage& image);

	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

private:
	void pickPhysicalDevice(VkInstance vulkan_instance, VkSurfaceKHR surface);

	void createLogicalDevice(VkSurfaceKHR surface, QueueFamilyIndices& indices);

	void createCommandPool(QueueFamilyIndices& indices);
};

bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);

bool checkDeviceExtensionSupport(VkPhysicalDevice device);