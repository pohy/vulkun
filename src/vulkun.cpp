#include "vulkun.h"
#include "pipeline_builder.h"
#include "vk_initializers.h"
#include "vk_types.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#define VK_KHR_portability_enumeration 1
#include <VkBootstrap.h>

#include <cassert>
#include <chrono>
#include <fstream>
#include <thread>

constexpr bool enable_validation_layers = true;
Vulkun *singleton_instance = nullptr;

Vulkun &Vulkun::get_singleton() {
	return *singleton_instance;
}

void Vulkun::init() {
	// TODO: I don't actually understand why do we initialize the instance here, instead of the `get_singleton` method.
	assert(singleton_instance == nullptr);
	singleton_instance = this;

	// Initialize SDL
	// TODO: SDL validations
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);
	_window = SDL_CreateWindow(APP_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _window_extent.width, _window_extent.height, window_flags);

	_is_initialized = _init_vulkan();
	_is_initialized = _init_swapchain();
	_is_initialized = _init_commands();
	_is_initialized = _init_default_renderpass();
	_is_initialized = _init_framebuffers();
	_is_initialized = _init_sync_structures();
	_is_initialized = _init_pipelines();

	fmt::print("Vulkun initialized: {}\n", _is_initialized);
}

bool Vulkun::_init_vulkan() {
	vkb::InstanceBuilder builder;

	auto inst_ret = builder
							.set_app_name(APP_NAME)
							.require_api_version(1, 1, 0)
							.request_validation_layers(enable_validation_layers)
							.use_default_debug_messenger()
							.build();

	if (!inst_ret) {
		fmt::print(stderr, "Failed to create Vulkan instance. Error: {}\n", inst_ret.error().message().c_str());
		return false;
	}

	vkb::Instance vkb_inst = inst_ret.value();

	_instance = vkb_inst.instance;
	_debug_messenger = vkb_inst.debug_messenger;

	if (!SDL_Vulkan_CreateSurface(_window, _instance, &_surface)) {
		fmt::print(stderr, "Failed to create Vulkan surface.\n");
		return false;
	}

	vkb::PhysicalDeviceSelector selector(vkb_inst);
	vkb::PhysicalDevice physical_device = selector
												  .set_surface(_surface)
												  .set_minimum_version(1, 2)
												  .select()
												  .value();
	vkb::DeviceBuilder device_builder(physical_device);
	vkb::Device vkb_device = device_builder.build().value();

	_device = vkb_device.device;
	_physical_device = physical_device.physical_device;

	_graphics_queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
	_graphics_queue_family_idx = vkb_device.get_queue_index(vkb::QueueType::graphics).value();

	return true;
}

bool Vulkun::_init_swapchain() {
	vkb::SwapchainBuilder swapchain_builder(_physical_device, _device, _surface);
	vkb::Swapchain vkb_swapchain = swapchain_builder
										   .use_default_format_selection()
										   .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
										   .set_desired_extent(_window_extent.width, _window_extent.height)
										   .build()
										   .value();

	_swapchain = vkb_swapchain.swapchain;
	_swapchain_images = vkb_swapchain.get_images().value();
	_swapchain_image_views = vkb_swapchain.get_image_views().value();
	_swapchain_image_format = vkb_swapchain.image_format;

	return true;
}

bool Vulkun::_init_commands() {
	VkCommandPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.pNext = nullptr;
	pool_info.queueFamilyIndex = _graphics_queue_family_idx;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK_CHECK(vkCreateCommandPool(_device, &pool_info, nullptr, &_command_pool));

	VkCommandBufferAllocateInfo cmd_alloc_info = {};
	cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd_alloc_info.pNext = nullptr;
	cmd_alloc_info.commandPool = _command_pool;
	cmd_alloc_info.commandBufferCount = 1;
	cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	VK_CHECK(vkAllocateCommandBuffers(_device, &cmd_alloc_info, &_main_command_buffer));

	return true;
}

bool Vulkun::_init_default_renderpass() {
	VkAttachmentDescription color_attachment = {};
	color_attachment.format = _swapchain_image_format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.pNext = nullptr;
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;

	VK_CHECK(vkCreateRenderPass(_device, &render_pass_info, nullptr, &_render_pass));

	return true;
}

bool Vulkun::_init_framebuffers() {
	VkFramebufferCreateInfo framebuffer_info = {};
	framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_info.pNext = nullptr;
	framebuffer_info.renderPass = _render_pass;
	framebuffer_info.attachmentCount = 1;
	framebuffer_info.width = _window_extent.width;
	framebuffer_info.height = _window_extent.height;
	framebuffer_info.layers = 1;

	const uint32_t swapchain_image_count = _swapchain_images.size();
	_framebuffers.resize(swapchain_image_count);
	for (uint32_t i = 0; i < swapchain_image_count; i++) {
		framebuffer_info.pAttachments = &_swapchain_image_views[i];
		VK_CHECK(vkCreateFramebuffer(_device, &framebuffer_info, nullptr, &_framebuffers[i]));
	}

	return true;
}

bool Vulkun::_init_sync_structures() {
	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.pNext = nullptr;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VK_CHECK(vkCreateFence(_device, &fence_info, nullptr, &_render_fence));

	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_info.pNext = nullptr;

	VK_CHECK(vkCreateSemaphore(_device, &semaphore_info, nullptr, &_present_semaphore));
	VK_CHECK(vkCreateSemaphore(_device, &semaphore_info, nullptr, &_render_semaphore));

	return true;
}

bool Vulkun::_init_pipelines() {
	bool success = false;

	std::vector<std::string> shaders_names = {
		"triangle", "colored_triangle"
	};

	for (auto &shader_name : shaders_names) {
		VkShaderModule vert_shader_module, frag_shader_module;
		success = _load_shader_module(fmt::format("shaders/{}.vert.spv", shader_name).c_str(), &vert_shader_module);
		success = _load_shader_module(fmt::format("shaders/{}.frag.spv", shader_name).c_str(), &frag_shader_module);

		VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();
		VK_CHECK(vkCreatePipelineLayout(_device, &pipeline_layout_info, nullptr, &_triangle_pipeline_layout));

		PipelineBuilder pipeline_builder;
		pipeline_builder.shader_stages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, vert_shader_module));
		pipeline_builder.shader_stages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, frag_shader_module));
		pipeline_builder.vertex_input_info = vkinit::vertex_input_state_create_info();
		pipeline_builder.input_assembly = vkinit::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		pipeline_builder.viewport = {
			.x = 0.0f,
			.y = 0.0f,
			.width = (float)_window_extent.width,
			.height = (float)_window_extent.height,
			.minDepth = 0.0f,
			.maxDepth = 1.0f,
		};
		pipeline_builder.scissor = {
			.offset = { 0, 0 },
			.extent = _window_extent,
		};
		pipeline_builder.rasterizer = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL);
		pipeline_builder.multisampling = vkinit::multisampling_state_create_info();
		pipeline_builder.color_blend_attachment = vkinit::color_blend_attachment_state();
		pipeline_builder.pipeline_layout = _triangle_pipeline_layout;

		_pipelines.push_back(pipeline_builder.build_pipeline(_device, _render_pass));
	}

	// VkShaderModule triangle_vert_shader, triangle_frag_shader;
	// success = _load_shader_module("shaders/colored_triangle.vert.spv", &triangle_vert_shader);
	// success = _load_shader_module("shaders/colored_triangle.frag.spv", &triangle_frag_shader);

	// VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();
	// VK_CHECK(vkCreatePipelineLayout(_device, &pipeline_layout_info, nullptr, &_triangle_pipeline_layout));
	//
	// PipelineBuilder pipeline_builder;
	// pipeline_builder.shader_stages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, triangle_vert_shader));
	// pipeline_builder.shader_stages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, triangle_frag_shader));
	// pipeline_builder.vertex_input_info = vkinit::vertex_input_state_create_info();
	// pipeline_builder.input_assembly = vkinit::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	// pipeline_builder.viewport = {
	// 	.x = 0.0f,
	// 	.y = 0.0f,
	// 	.width = (float)_window_extent.width,
	// 	.height = (float)_window_extent.height,
	// 	.minDepth = 0.0f,
	// 	.maxDepth = 1.0f,
	// };
	// pipeline_builder.scissor = {
	// 	.offset = { 0, 0 },
	// 	.extent = _window_extent,
	// };
	// pipeline_builder.rasterizer = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL);
	// pipeline_builder.multisampling = vkinit::multisampling_state_create_info();
	// pipeline_builder.color_blend_attachment = vkinit::color_blend_attachment_state();
	// pipeline_builder.pipeline_layout = _triangle_pipeline_layout;
	// _triangle_pipeline = pipeline_builder.build_pipeline(_device, _render_pass);

	return success;
}

bool Vulkun::_load_shader_module(const char *file_path, VkShaderModule *out_shader_module) {
	// Load the shader file
	std::ifstream file(file_path, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		fmt::println(stderr, "Failed to open file: {}", file_path);
		return false;
	}

	size_t file_size = (size_t)file.tellg();
	std::vector<uint32_t> buffer(file_size / sizeof(uint32_t));

	file.seekg(0);
	file.read((char *)buffer.data(), file_size);
	file.close();

	// Create the shader module
	VkShaderModuleCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.pNext = nullptr;
	create_info.codeSize = buffer.size() * sizeof(uint32_t);
	create_info.pCode = buffer.data();

	VkShaderModule shader_module;
	if (vkCreateShaderModule(_device, &create_info, nullptr, &shader_module) != VK_SUCCESS) {
		fmt::println(stderr, "Failed to create shader module. At path: {}", file_path);
		return false;
	}

	fmt::print("Shader module created: {}\n", file_path);

	*out_shader_module = shader_module;

	return true;
}

void Vulkun::run() {
	bool should_quit = false;
	SDL_Event event;

	while (!should_quit) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				should_quit = true;
			}

			if (event.type == SDL_WINDOWEVENT) {
				if (event.window.event == SDL_WINDOW_MINIMIZED) {
					_is_rendering_paused = true;
				}
				if (event.window.event == SDL_WINDOW_MAXIMIZED) {
					_is_rendering_paused = false;
				}
			}

			if (event.type == SDL_KEYUP) {
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					should_quit = true;
				}
				if (event.key.keysym.sym == SDLK_SPACE) {
					_selected_pipeline_idx = (_selected_pipeline_idx + 1) % _pipelines.size();
				}
			}
		}

		if (should_quit) {
			break;
		}

		if (_is_rendering_paused) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		draw();
	}
}

void Vulkun::draw() {
	VK_CHECK(vkWaitForFences(_device, 1, &_render_fence, true, 1000000000));
	VK_CHECK(vkResetFences(_device, 1, &_render_fence));

	VK_CHECK(vkResetCommandBuffer(_main_command_buffer, 0));

	uint32_t swapchain_image_index;
	VK_CHECK(vkAcquireNextImageKHR(_device, _swapchain, 1000000000, _present_semaphore, nullptr, &swapchain_image_index));

	VkCommandBufferBeginInfo cmd_begin_info = {};
	cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmd_begin_info.pNext = nullptr;
	cmd_begin_info.pInheritanceInfo = nullptr;
	cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VK_CHECK(vkBeginCommandBuffer(_main_command_buffer, &cmd_begin_info));

	VkClearValue clear_color;
	float flash = abs(sin(_frame_number / 120.0f));
	clear_color.color = { { 1.0f - flash, flash, flash * 0.5f, 1.0f } };

	VkRenderPassBeginInfo render_pass_begin_info = {};
	render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.pNext = nullptr;
	render_pass_begin_info.renderPass = _render_pass;
	render_pass_begin_info.renderArea.offset.x = 0;
	render_pass_begin_info.renderArea.offset.y = 0;
	render_pass_begin_info.renderArea.extent = _window_extent;
	render_pass_begin_info.framebuffer = _framebuffers[swapchain_image_index];
	render_pass_begin_info.clearValueCount = 1;
	render_pass_begin_info.pClearValues = &clear_color;

	vkCmdBeginRenderPass(_main_command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

	if (_selected_pipeline_idx >= 0 && _selected_pipeline_idx < _pipelines.size()) {
		vkCmdBindPipeline(_main_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelines[_selected_pipeline_idx]);
		vkCmdDraw(_main_command_buffer, 3, 1, 0, 0);
	} else {
		fmt::println(stderr, "Pipeline index out of bounds: {}/{}", _selected_pipeline_idx, _pipelines.size());
	}

	vkCmdEndRenderPass(_main_command_buffer);
	VK_CHECK(vkEndCommandBuffer(_main_command_buffer));

	VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = nullptr;
	submit_info.pWaitDstStageMask = &wait_stage;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = &_present_semaphore;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &_render_semaphore;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &_main_command_buffer;

	VK_CHECK(vkQueueSubmit(_graphics_queue, 1, &submit_info, _render_fence));

	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.pNext = nullptr;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &_render_semaphore;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &_swapchain;
	present_info.pImageIndices = &swapchain_image_index;

	VK_CHECK(vkQueuePresentKHR(_graphics_queue, &present_info));

	_frame_number++;
}

void Vulkun::cleanup() {
	singleton_instance = nullptr;

	if (!_is_initialized) {
		return;
	}

	VK_CHECK(vkWaitForFences(_device, 1, &_render_fence, true, 1000000000));

	vkDestroySemaphore(_device, _render_semaphore, nullptr);
	vkDestroySemaphore(_device, _present_semaphore, nullptr);
	vkDestroyFence(_device, _render_fence, nullptr);

	for (int i = 0; i < _framebuffers.size(); i++) {
		vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);
		vkDestroyImageView(_device, _swapchain_image_views[i], nullptr);
	}

	vkDestroyRenderPass(_device, _render_pass, nullptr);

	vkDestroyCommandPool(_device, _command_pool, nullptr);

	vkDestroySwapchainKHR(_device, _swapchain, nullptr);

	vkDestroyDevice(_device, nullptr);
	vkDestroySurfaceKHR(_instance, _surface, nullptr);

	vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
	vkDestroyInstance(_instance, nullptr);

	SDL_DestroyWindow(_window);
}
