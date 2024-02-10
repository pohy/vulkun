#pragma once

#include <array>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>
// #include <vk_mem_alloc.h>

#define FMT_HEADER_ONLY
#include <fmt/core.h>

// #include <glm/mat4x4.hpp>
// #include <glm/vec4.hpp>

#define VK_CHECK(x)                                                        \
	do {                                                                   \
		VkResult err = x;                                                  \
		if (err) {                                                         \
			fmt::print("Detected Vulkan error: {}", string_VkResult(err)); \
			abort();                                                       \
		}                                                                  \
	} while (0)
