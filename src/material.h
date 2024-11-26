#pragma once

#include "vk_types.h"
#include "vulkan/vulkan_core.h"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <string>

namespace MaterialName {
const std::string Default = "default";
const std::string ShiftingColors = "shifting_colors";
const std::string Impreza = "impreza";
const std::string Lit = "default_lit";
} //namespace MaterialName

struct GPUMaterialUniforms {
	glm::vec4 albedo_color{ 1.0f, 1.0f, 1.0f, 1.0f };
};

struct Material {
	VkPipeline pipeline;
	VkPipelineLayout pipeline_layout;

	const VkDescriptorSetLayout *pDescriptor_set_layout;
	VkDescriptorSet descriptor_set;
	AllocatedBuffer uniform_buffer;

	GPUMaterialUniforms uniforms;
};
