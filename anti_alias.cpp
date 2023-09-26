#include "anti_alias.h"

MSAA::MSAA(VkPhysicalDevice physical_device, VkDevice logical_device, MemoryAllocator* mem_allocator, ImageCreator* img_creator) {
	msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	physicalDevice = physical_device;
	device = logical_device;
	memAllocator = mem_allocator;
	imageCreator = img_creator;

	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physical_device, &physicalDeviceProperties);
	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT) max_msaaSamples = VK_SAMPLE_COUNT_64_BIT;
	else if (counts & VK_SAMPLE_COUNT_32_BIT) max_msaaSamples = VK_SAMPLE_COUNT_32_BIT;
	else if (counts & VK_SAMPLE_COUNT_16_BIT) max_msaaSamples = VK_SAMPLE_COUNT_16_BIT;
	else if (counts & VK_SAMPLE_COUNT_8_BIT) max_msaaSamples = VK_SAMPLE_COUNT_8_BIT;
	else if (counts & VK_SAMPLE_COUNT_4_BIT) max_msaaSamples = VK_SAMPLE_COUNT_4_BIT;
	else if (counts & VK_SAMPLE_COUNT_2_BIT) max_msaaSamples = VK_SAMPLE_COUNT_2_BIT;
	else max_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
}

void MSAA::destroyColorResources() {
	vkDestroyImageView(device, colorImageView, nullptr);
	vkDestroyImage(device, colorImage, nullptr);
	vkFreeMemory(device, colorImageMemory, nullptr);
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
	imageCreator->createImage(swapChainExtent.width, swapChainExtent.height, msaaSamples, swapChainImageFormat, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, colorImage);
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, colorImage, &memRequirements);
	memAllocator->allocateMemory(memRequirements.size, memAllocator->findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT), colorImageMemory);
	vkBindImageMemory(device, colorImage, colorImageMemory, 0);
	colorImageView = imageCreator->createImageView(colorImage, swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
}

VkImageView MSAA::getColorImageView() {
	return colorImageView;
}