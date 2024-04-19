#pragma once

#include "gpu.h"

class MSAA {
private:
	VkSampleCountFlagBits msaaSamples;
	GPU* gpu;
	VkSampleCountFlagBits max_msaaSamples;
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

public:
	MSAA(GPU* gpu_);
	VkSampleCountFlagBits getMaxUsableSampleCount();
	VkSampleCountFlagBits getSampleCount();
	void setSampleCount(VkSampleCountFlagBits sample_count);
	void createColorResources(VkFormat swapChainImageFormat, VkExtent2D swapChainExtent);
	void destroyColorResources();
	VkImageView getColorImageView();
};