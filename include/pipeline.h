#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

#include "gpu.h"
#include "anti_alias.h"

class Pipeline {
public:
	VkPipeline pipeline;
	VkPipelineLayout layout;

	Pipeline();
	void create(
		GPU* gpu, MSAA* msaa, VkRenderPass render_pass,
		std::string vertex_shader, std::string fragment_shader,
		VkVertexInputBindingDescription bindingDescription,
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions,
		std::vector<VkDescriptorSetLayout>& setLayouts
	);
};

std::vector<char> readFile(const std::string& filename);