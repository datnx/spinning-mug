#include "anti_alias.h"

MSAA::MSAA(GPU* gpu_, VkFormat swapChainImageFormat, VkExtent2D swapChainExtent) {
	msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	gpu = gpu_;

	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(gpu->physical_gpu, &physicalDeviceProperties);
	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT) max_msaaSamples = VK_SAMPLE_COUNT_64_BIT;
	else if (counts & VK_SAMPLE_COUNT_32_BIT) max_msaaSamples = VK_SAMPLE_COUNT_32_BIT;
	else if (counts & VK_SAMPLE_COUNT_16_BIT) max_msaaSamples = VK_SAMPLE_COUNT_16_BIT;
	else if (counts & VK_SAMPLE_COUNT_8_BIT) max_msaaSamples = VK_SAMPLE_COUNT_8_BIT;
	else if (counts & VK_SAMPLE_COUNT_4_BIT) max_msaaSamples = VK_SAMPLE_COUNT_4_BIT;
	else if (counts & VK_SAMPLE_COUNT_2_BIT) max_msaaSamples = VK_SAMPLE_COUNT_2_BIT;
	else max_msaaSamples = VK_SAMPLE_COUNT_1_BIT;

	msaaSamples = (max_msaaSamples >= VK_SAMPLE_COUNT_4_BIT) ? VK_SAMPLE_COUNT_4_BIT : max_msaaSamples;

	createColorResources(swapChainImageFormat, swapChainExtent);
}

void MSAA::destroyColorResources() {
	vkDestroyImageView(gpu->logical_gpu, colorImageView, nullptr);
	vkDestroyImage(gpu->logical_gpu, colorImage, nullptr);
	vkFreeMemory(gpu->logical_gpu, colorImageMemory, nullptr);
}

VkSampleCountFlagBits MSAA::getMaxUsableSampleCount() {
	return max_msaaSamples;
}

VkSampleCountFlagBits MSAA::getSampleCount() {
	return msaaSamples;
}

void MSAA::setSampleCount(VkSampleCountFlagBits sample_count) {
	msaaSamples = sample_count;
}

void MSAA::createColorResources(VkFormat swapChainImageFormat, VkExtent2D swapChainExtent) {
	gpu->createImage(swapChainExtent.width, swapChainExtent.height, msaaSamples, swapChainImageFormat,
		VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, colorImage);
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(gpu->logical_gpu, colorImage, &memRequirements);

	gpu->allocateMemory(memRequirements.size, gpu->findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT), colorImageMemory);
	vkBindImageMemory(gpu->logical_gpu, colorImage, colorImageMemory, 0);
	colorImageView = gpu->createImageView(colorImage, swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
}

VkImageView MSAA::getColorImageView() {
	return colorImageView;
}