#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>

#define FMT_HEADER_ONLY
#include <fmt/core.h>

struct AllocatedBuffer {
	VkBuffer buffer;
	VmaAllocation allocation;
};

struct AllocatedImage {
	VkImage image;
	VmaAllocation allocation;
};

// TODO: Print stacktrace on error
#define VK_CHECK(param)                                                             \
	if (param) {                                                                    \
		VkResult err = param;                                                       \
		fmt::println(stderr, "🔥 Detected Vulkan error: {}", string_VkResult(err)); \
		abort();                                                                    \
	} else                                                                          \
		((void)0)

#define ASSERT_MSG(param, msg)              \
	if (!(param)) {                         \
		fmt::println(stderr, "🔥 {}", msg); \
		abort();                            \
	} else                                  \
		((void)0)\
