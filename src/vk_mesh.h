#pragma once

#include "vk_types.h"

#include <glm/vec3.hpp>
#include <vector>

struct VertexInputDescription {
	std::vector<VkVertexInputBindingDescription> bindings;
	std::vector<VkVertexInputAttributeDescription> attributes;
	VkPipelineVertexInputStateCreateFlags flags = 0;
};

struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec3 color;

	static VertexInputDescription create_vertex_description();
};

struct Mesh {
	std::vector<Vertex> vertices;
	AllocatedBuffer vertexBuffer;

	size_t size_of_vertices() {
		return vertices.size() * sizeof(Vertex);
	}
};
