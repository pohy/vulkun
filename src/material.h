#pragma once

#include <vulkan/vulkan.h>
#include <string>

namespace MaterialName {
const std::string Default = "default";
}

struct Material {
	VkPipeline pipeline;
	VkPipelineLayout pipeline_layout;
};
