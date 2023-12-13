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
	void set_gpu(GPU* g);
	void set_shader_codes(std::string vertex, std::string fragment);
	void create(MSAA* msaa, VkRenderPass render_pass,
		std::vector<VkDescriptorSetLayout>& setLayouts);

private:
	GPU* gpu;
	std::string vertex_shader_code;
	std::string fragment_shader_code;
};

std::vector<char> readFile(const std::string& filename);