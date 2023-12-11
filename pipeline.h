#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

class Pipeline {
public:
	VkPipeline pipeline;
	VkPipelineLayout layout;

	Pipeline();
};

std::vector<char> readFile(const std::string& filename);