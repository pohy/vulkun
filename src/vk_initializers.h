#pragma once

#include "vk_types.h"
#include "vulkan/vulkan_core.h"

namespace vkinit {
VkCommandPoolCreateInfo command_pool_create_info(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
VkCommandBufferAllocateInfo command_buffer_allocate_info(VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info(VkShaderStageFlagBits stage_flags, VkShaderModule shader_module);
VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info();
VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info(VkPrimitiveTopology topology);
VkPipelineRasterizationStateCreateInfo rasterization_state_create_info(VkPolygonMode polygon_mode);
VkPipelineMultisampleStateCreateInfo multisampling_state_create_info();
VkPipelineColorBlendAttachmentState color_blend_attachment_state();
VkPipelineLayoutCreateInfo pipeline_layout_create_info();
VkImageCreateInfo image_create_info(VkFormat format, VkImageUsageFlags usage_flags, VkExtent3D extent);
VkImageViewCreateInfo image_view_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspect_flags);
VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info(bool shuold_depth_test, bool should_depth_write, VkCompareOp compare_op);
VkDescriptorSetLayoutBinding descriptor_set_layout_binding(VkDescriptorType type, VkShaderStageFlags stage_flags, uint32_t binding);
VkWriteDescriptorSet write_descriptor_buffer(VkDescriptorType type, VkDescriptorSet dst_set, VkDescriptorBufferInfo *buffer_info, uint32_t binding);
} //namespace vkinit
