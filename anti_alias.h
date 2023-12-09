#pragma once

#include "gpu.h"
#include "image.h"

class MSAA {
private:
	VkSampleCountFlagBits msaaSamples;
	GPU* gpu;
	VkSampleCountFlagBits max_msaaSamples;
	ImageCreator* imageCreator;
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

public:
	MSAA(GPU* gpu_, ImageCreator* img_creator);
	VkSampleCountFlagBits getMaxUsableSampleCount();
	VkSampleCountFlagBits getSampleCount();
	void setSampleCount(VkSampleCountFlagBits sample_count);
	void createColorResources(VkFormat swapChainImageFormat, VkExtent2D swapChainExtent);
	void destroyColorResources();
	VkImageView getColorImageView();
};