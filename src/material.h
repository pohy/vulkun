#pragma once

#include <vulkan/vulkan.h>
#include <string>

namespace MaterialName {
const std::string Default = "default";
const std::string ShiftingColors = "shifting_colors";
const std::string Impreza = "impreza";
} //namespace MaterialName

struct Material {
	VkPipeline pipeline;
	VkPipelineLayout pipeline_layout;
};
