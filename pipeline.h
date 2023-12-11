#pragma once

#include <vulkan/vulkan.h>

class Pipeline {
public:
	VkPipeline pipeline;
	VkPipelineLayout layout;

	Pipeline();
};