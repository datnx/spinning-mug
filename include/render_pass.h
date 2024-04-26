#pragma once

#include <vulkan/vulkan.h>

class RenderPass {
private:
	VkRenderPass renderPass;
	VkDevice device;

public:
	RenderPass(VkDevice d, VkFormat color_format, VkFormat depth_format, VkSampleCountFlagBits msaaSamples);
	~RenderPass();
	VkRenderPass getRenderPass();
};