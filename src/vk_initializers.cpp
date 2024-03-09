#include "vk_initializers.h"
#include "vulkan/vulkan_core.h"

VkCommandPoolCreateInfo vkinit::command_pool_create_info(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags) {
	VkCommandPoolCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.pNext = nullptr;
	info.queueFamilyIndex = queueFamilyIndex;
	info.flags = flags;

	return info;
}

VkCommandBufferAllocateInfo vkinit::command_buffer_allocate_info(VkCommandPool pool, uint32_t count, VkCommandBufferLevel level) {
	VkCommandBufferAllocateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.pNext = nullptr;
	info.commandPool = pool;
	info.commandBufferCount = count;
	info.level = level;

	return info;
}

VkPipelineShaderStageCreateInfo vkinit::pipeline_shader_stage_create_info(VkShaderStageFlagBits stage_flags, VkShaderModule shader_module) {
	VkPipelineShaderStageCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	info.pNext = nullptr;
	info.stage = stage_flags;
	info.module = shader_module;
	// Shader entry point aka. the "main" function
	info.pName = "main";

	return info;
}

VkPipelineVertexInputStateCreateInfo vkinit::vertex_input_state_create_info() {
	VkPipelineVertexInputStateCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	info.pNext = nullptr;
	info.vertexBindingDescriptionCount = 0;
	info.vertexAttributeDescriptionCount = 0;

	return info;
}

VkPipelineInputAssemblyStateCreateInfo vkinit::input_assembly_create_info(VkPrimitiveTopology topology) {
	VkPipelineInputAssemblyStateCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	info.pNext = nullptr;
	info.topology = topology;
	info.primitiveRestartEnable = VK_FALSE;

	return info;
}

VkPipelineRasterizationStateCreateInfo vkinit::rasterization_state_create_info(VkPolygonMode polygon_mode) {
	VkPipelineRasterizationStateCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	info.pNext = nullptr;
	info.depthClampEnable = VK_FALSE;
	// Useful for when the shader writes to other outputs and we want to skip the rasterization and following stages
	info.rasterizerDiscardEnable = VK_FALSE;
	info.polygonMode = polygon_mode;
	info.lineWidth = 1.0f;
	info.cullMode = VK_CULL_MODE_NONE;
	info.frontFace = VK_FRONT_FACE_CLOCKWISE;
	info.depthBiasEnable = VK_FALSE;
	info.depthBiasConstantFactor = 0.0f;
	info.depthBiasClamp = 0.0f;
	info.depthBiasSlopeFactor = 0.0f;

	return info;
}

VkPipelineMultisampleStateCreateInfo vkinit::multisampling_state_create_info() {
	VkPipelineMultisampleStateCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	info.pNext = nullptr;
	info.sampleShadingEnable = VK_FALSE;
	info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	info.minSampleShading = 1.0f;
	info.pSampleMask = nullptr;
	info.alphaToCoverageEnable = VK_FALSE;
	info.alphaToOneEnable = VK_FALSE;

	return info;
}

VkPipelineColorBlendAttachmentState vkinit::color_blend_attachment_state() {
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	return colorBlendAttachment;
}

VkPipelineLayoutCreateInfo vkinit::pipeline_layout_create_info() {
	VkPipelineLayoutCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.setLayoutCount = 0;
	info.pSetLayouts = nullptr;
	info.pushConstantRangeCount = 0;
	info.pPushConstantRanges = nullptr;

	return info;
}

VkImageCreateInfo vkinit::image_create_info(VkFormat format, VkImageUsageFlags usage_flags, VkExtent3D extent) {
	VkImageCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	info.pNext = nullptr;
	info.imageType = VK_IMAGE_TYPE_2D;
	info.format = format;
	info.extent = extent;
	info.mipLevels = 1;
	info.arrayLayers = 1;
	info.samples = VK_SAMPLE_COUNT_1_BIT;
	info.tiling = VK_IMAGE_TILING_OPTIMAL;
	info.usage = usage_flags;

	return info;
}

VkImageViewCreateInfo vkinit::image_view_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspect_flags) {
	VkImageViewCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	info.pNext = nullptr;
	info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	info.format = format;
	info.image = image;
	info.subresourceRange.aspectMask = aspect_flags;
	info.subresourceRange.baseMipLevel = 0;
	info.subresourceRange.levelCount = 1;
	info.subresourceRange.baseArrayLayer = 0;
	info.subresourceRange.layerCount = 1;

	return info;
}

VkPipelineDepthStencilStateCreateInfo vkinit::depth_stencil_create_info(bool shuold_depth_test, bool should_depth_write, VkCompareOp compare_op) {
	VkPipelineDepthStencilStateCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	info.pNext = nullptr;
	info.depthTestEnable = shuold_depth_test ? VK_TRUE : VK_FALSE;
	info.depthWriteEnable = should_depth_write ? VK_TRUE : VK_FALSE;
	info.depthCompareOp = shuold_depth_test ? compare_op : VK_COMPARE_OP_ALWAYS;
	info.depthBoundsTestEnable = VK_FALSE;
	info.minDepthBounds = 0.0f;
	info.maxDepthBounds = 1.0f;
	info.stencilTestEnable = VK_FALSE;

	return info;
}

VkDescriptorSetLayoutBinding vkinit::descriptor_set_layout_binding(VkDescriptorType type, VkShaderStageFlags stage_flags, uint32_t binding) {
	VkDescriptorSetLayoutBinding set_binding{};
	set_binding.descriptorCount = 1;
	set_binding.pImmutableSamplers = nullptr;
	set_binding.binding = binding;
	set_binding.descriptorType = type;
	set_binding.stageFlags = stage_flags;

	return set_binding;
}

VkWriteDescriptorSet vkinit::write_descriptor_buffer(VkDescriptorType type, VkDescriptorSet dst_set, VkDescriptorBufferInfo *buffer_info, uint32_t binding) {
	VkWriteDescriptorSet write_set{};
	write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_set.pNext = nullptr;

	write_set.descriptorCount = 1;
	write_set.dstSet = dst_set;
	write_set.dstBinding = binding;
	write_set.descriptorType = type;
	write_set.pBufferInfo = buffer_info;

	return write_set;
}
