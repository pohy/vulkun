#include "pipeline_builder.h"
#include "mesh.h"
#include "vk_initializers.h"

PipelineBuilder PipelineBuilder::create_vert_frag_pipeline(VkShaderModule vert_shader, VkShaderModule frag_shader, VkExtent2D extent) {
	PipelineBuilder pipeline_builder;

	pipeline_builder.shader_stages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, vert_shader));
	pipeline_builder.shader_stages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, frag_shader));

	pipeline_builder.vertex_input_info = vkinit::vertex_input_state_create_info();
	pipeline_builder.input_assembly = vkinit::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipeline_builder.viewport = {
		.x = 0.0f,
		.y = 0.0f,
		.width = (float)extent.width,
		.height = (float)extent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};
	pipeline_builder.scissor = {
		.offset = { 0, 0 },
		.extent = extent,
	};
	pipeline_builder.rasterizer = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL);
	pipeline_builder.multisampling = vkinit::multisampling_state_create_info();
	pipeline_builder.color_blend_attachment = vkinit::color_blend_attachment_state();

	pipeline_builder.vertex_input_description = Vertex::create_vertex_description();

	pipeline_builder.vertex_input_info.flags = pipeline_builder.vertex_input_description.flags;

	pipeline_builder.vertex_input_info.vertexBindingDescriptionCount = pipeline_builder.vertex_input_description.bindings.size();
	pipeline_builder.vertex_input_info.pVertexBindingDescriptions = pipeline_builder.vertex_input_description.bindings.data();

	fmt::println("\tVertex binding description count: {}", pipeline_builder.vertex_input_info.vertexBindingDescriptionCount);

	pipeline_builder.vertex_input_info.vertexAttributeDescriptionCount = pipeline_builder.vertex_input_description.attributes.size();
	pipeline_builder.vertex_input_info.pVertexAttributeDescriptions = pipeline_builder.vertex_input_description.attributes.data();

	fmt::println("\tVertex attribute description count: {}", pipeline_builder.vertex_input_info.vertexAttributeDescriptionCount);

	pipeline_builder.depth_stencil = vkinit::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

	return pipeline_builder;
}

VkPipeline PipelineBuilder::build_pipeline(VkDevice device, VkRenderPass render_pass) {
	VkPipelineViewportStateCreateInfo viewport_state{};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.pNext = nullptr;
	viewport_state.viewportCount = 1;
	viewport_state.pViewports = &viewport;
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = &scissor;

	VkPipelineColorBlendStateCreateInfo color_blending{};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.pNext = nullptr;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;

	VkGraphicsPipelineCreateInfo pipeline_info{};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.pNext = nullptr;
	pipeline_info.stageCount = shader_stages.size();
	pipeline_info.pStages = shader_stages.data();
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.pDepthStencilState = &depth_stencil;
	pipeline_info.layout = pipeline_layout;
	pipeline_info.renderPass = render_pass;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

	VkPipeline pipeline;
	VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline);
	if (result != VK_SUCCESS) {
		fmt::println("Failed to create pipeline: {}", string_VkResult(result));
		return VK_NULL_HANDLE;
	}

	return pipeline;
}
