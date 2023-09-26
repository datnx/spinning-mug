#pragma once

#include <vulkan/vulkan.h>

#include "image.h"

class MSAA {
private:
	VkSampleCountFlagBits msaaSamples;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	MemoryAllocator* memAllocator;
	VkSampleCountFlagBits max_msaaSamples;
	ImageCreator* imageCreator;
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

public:
	MSAA(VkPhysicalDevice physical_device, VkDevice logical_device, MemoryAllocator* mem_allocator, ImageCreator* img_creator);
	VkSampleCountFlagBits getMaxUsableSampleCount();
	VkSampleCountFlagBits getSampleCount();
	void setSampleCount(VkSampleCountFlagBits sample_count);
	void createColorResources(VkFormat swapChainImageFormat, VkExtent2D swapChainExtent);
	void destroyColorResources();
	VkImageView getColorImageView();
};