#pragma once

#include "vk_types.h"

class PipelineBuilder {
public:
	std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
	VkPipelineVertexInputStateCreateInfo vertex_input_info;
	VkPipelineInputAssemblyStateCreateInfo input_assembly;
	VkViewport viewport;
	VkRect2D scissor;
	VkPipelineRasterizationStateCreateInfo rasterizer;
	VkPipelineColorBlendAttachmentState color_blend_attachment;
	VkPipelineDepthStencilStateCreateInfo depth_stencil;
	VkPipelineMultisampleStateCreateInfo multisampling;
	VkPipelineLayout pipeline_layout;

	VkPipeline build_pipeline(VkDevice device, VkRenderPass render_pass);
};
