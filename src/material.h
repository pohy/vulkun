#pragma once

#include <vulkan/vulkan.h>
#include <string>

namespace MaterialName {
const std::string Default = "default";
const std::string ShiftingColors = "shifting_colors";
} //namespace MaterialName

struct Material {
	VkPipeline pipeline;
	VkPipelineLayout pipeline_layout;
};
