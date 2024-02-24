#pragma once

#include "vk_types.h"

#include <glm/glm.hpp>

#include <vector>

namespace MeshName {
const std::string Triangle = "triangle";
const std::string Monkey = "monkey";
}

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

	bool load_from_obj(std::string file_path);

	size_t size_of_vertices() {
		auto size = vertices.size() * sizeof(Vertex);
		// fmt::println("Size of vertices: {} * {} = {}", vertices.size(), sizeof(Vertex), size);
		return size;
	}
};
