#pragma once

#include "vk_types.h"

#include <glm/glm.hpp>

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
	AllocatedBuffer vertex_buffer;

	size_t size_of_vertices() {
		auto size = vertices.size() * sizeof(Vertex);
		// fmt::println("Size of vertices: {} * {} = {}", vertices.size(), sizeof(Vertex), size);
		return size;
	}
};
