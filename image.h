#pragma once

#include <vulkan/vulkan.h>

class ImageCreator {
private:
	VkDevice device;
public:
	ImageCreator(VkDevice d);

	void createImage(uint32_t width, uint32_t height, VkSampleCountFlagBits numSamples, VkFormat format, VkImageUsageFlags usage, VkImage& image);

	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
};