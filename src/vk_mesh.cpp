#include "vk_mesh.h"

VertexInputDescription Vertex::create_vertex_description() {
	VertexInputDescription description = {};

	VkVertexInputBindingDescription main_binding = {};
	main_binding.binding = 0;
	main_binding.stride = sizeof(Vertex);
	main_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	description.bindings.push_back(main_binding);

	VkVertexInputAttributeDescription pos_attribute = {};
	pos_attribute.binding = main_binding.binding;
	pos_attribute.location = 0;
	pos_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	pos_attribute.offset = offsetof(Vertex, pos);

	description.attributes.push_back(pos_attribute);

	VkVertexInputAttributeDescription normal_attribute = {};
	normal_attribute.binding = main_binding.binding;
	normal_attribute.location = 1;
	normal_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	normal_attribute.offset = offsetof(Vertex, normal);

	description.attributes.push_back(normal_attribute);

	VkVertexInputAttributeDescription color_attribute = {};
	color_attribute.binding = main_binding.binding;
	color_attribute.location = 2;
	color_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	color_attribute.offset = offsetof(Vertex, color);

	description.attributes.push_back(color_attribute);

	return description;
}
