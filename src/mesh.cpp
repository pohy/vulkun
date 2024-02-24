#include "mesh.h"

#include <fmt/core.h>
#include <tiny_obj_loader.h>

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

bool Mesh::load_from_obj(std::string file_path) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn, error;

	tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &error, file_path.c_str());

	fmt::println("Loaded obj file \"{}\" with {} shapes and {} materials", file_path, shapes.size(), materials.size());

	if (!warn.empty()) {
		fmt::print("\tWarning while loading obj file \"{}\" {}", file_path, warn);
	}
	if (!error.empty()) {
		fmt::print(stderr, "\tError while loading obj file \"{}\" {}", file_path, error);
		return false;
	}

	for (size_t shape_idx = 0; shape_idx < shapes.size(); ++shape_idx) {
		size_t mesh_index_offset = 0;
		fmt::println("\tShape[{}]: \"{}\" with {} vertices", shape_idx, shapes[shape_idx].name, shapes[shape_idx].mesh.indices.size());

		for (size_t face_idx = 0; face_idx < shapes[shape_idx].mesh.num_face_vertices.size(); ++face_idx) {
			// Hard code for now, the mesh has to be triangulated. But it seems, that the loader does triangulation.
			size_t face_vert_count = 3;
			for (size_t vert_idx = 0; vert_idx < face_vert_count; ++vert_idx) {
				tinyobj::index_t idx = shapes[shape_idx].mesh.indices[mesh_index_offset + vert_idx];

				Vertex vertex = {
					.pos = {
							attrib.vertices[3 * idx.vertex_index + 0],
							attrib.vertices[3 * idx.vertex_index + 1], // * -1.0f,
							attrib.vertices[3 * idx.vertex_index + 2],
					},
					.normal = {
							attrib.normals[3 * idx.normal_index + 0],
							attrib.normals[3 * idx.normal_index + 1], // * -1.0f,
							attrib.normals[3 * idx.normal_index + 2],
					},
				};
				vertex.color = vertex.normal;
				vertices.push_back(vertex);
			}
			mesh_index_offset += face_vert_count;
		}
	}

	fmt::println("\tLoaded mesh from file \"{}\" with {} vertices", file_path, vertices.size());

	return true;
}
