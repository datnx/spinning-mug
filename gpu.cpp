#include "gpu.h"

GPU::GPU(VkPhysicalDevice physical, VkDevice logical) {
	physical_gpu = physical;
	logical_gpu = logical;
}