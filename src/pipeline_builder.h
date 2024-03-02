#pragma once

#include "vk_types.h"
#include "mesh.h"

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

	VertexInputDescription vertex_input_description;

	static PipelineBuilder create_vert_frag_pipeline(VkShaderModule vert_shader, VkShaderModule frag_shader, VkExtent2D extent);

	VkPipeline build_pipeline(VkDevice device, VkRenderPass render_pass);
};
