#include "vulkun.h"
#include "pipeline_builder.h"
#include "vk_initializers.h"
#include "vk_types.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <imgui.h>
#include <imgui_impl_osx.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>

#include <VkBootstrap.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>

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

	fmt::println("PushConstants size: {}", sizeof(PushConstants));

	// Initialize SDL
	// TODO: SDL validations
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);
	_window = SDL_CreateWindow(APP_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _window_extent.width, _window_extent.height, window_flags);

	_deletion_queue.push_function([=]() {
		SDL_DestroyWindow(_window);
	});

	_is_initialized = _init_vulkan();
	_is_initialized = _init_swapchain();
	_is_initialized = _init_commands();
	_is_initialized = _init_default_renderpass();
	_is_initialized = _init_framebuffers();
	_is_initialized = _init_sync_structures();
	_is_initialized = _init_pipelines();
	_is_initialized = _init_imgui();

	_load_meshes();
	_is_initialized = _init_scene();

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

	_deletion_queue.push_function([=]() {
		vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
		vkDestroyInstance(_instance, nullptr);
	});

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

	_deletion_queue.push_function([=]() {
		vkDestroyDevice(_device, nullptr);
		vkDestroySurfaceKHR(_instance, _surface, nullptr);
	});

	_graphics_queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
	_graphics_queue_family_idx = vkb_device.get_queue_index(vkb::QueueType::graphics).value();

	VmaAllocatorCreateInfo allocator_info = {};
	allocator_info.physicalDevice = _physical_device;
	allocator_info.device = _device;
	allocator_info.instance = _instance;
	vmaCreateAllocator(&allocator_info, &_allocator);

	_deletion_queue.push_function([=]() {
		vmaDestroyAllocator(_allocator);
	});

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

	_deletion_queue.push_function([=]() {
		vkDestroySwapchainKHR(_device, _swapchain, nullptr);
	});

	VkExtent3D depth_image_extent = { _window_extent.width, _window_extent.height, 1 };
	VkImageCreateInfo depth_image_info = vkinit::image_create_info(_depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depth_image_extent);

	VmaAllocationCreateInfo depth_image_alloc_info = {};
	depth_image_alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	depth_image_alloc_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vmaCreateImage(_allocator, &depth_image_info, &depth_image_alloc_info, &_depth_image.image, &_depth_image.allocation, nullptr);

	VkImageViewCreateInfo depth_image_view_info = vkinit::image_view_create_info(_depth_format, _depth_image.image, VK_IMAGE_ASPECT_DEPTH_BIT);

	VK_CHECK(vkCreateImageView(_device, &depth_image_view_info, nullptr, &_depth_image_view));

	_deletion_queue.push_function([=]() {
		vkDestroyImageView(_device, _depth_image_view, nullptr);
		vmaDestroyImage(_allocator, _depth_image.image, _depth_image.allocation);
	});

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

	_deletion_queue.push_function([=]() {
		vkDestroyCommandPool(_device, _command_pool, nullptr);
	});

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

	VkAttachmentDescription depth_attachment = {};
	depth_attachment.format = _depth_format;
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_ref = {};
	depth_attachment_ref.attachment = 1;
	depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	subpass.pDepthStencilAttachment = &depth_attachment_ref;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency depth_dependency = {};
	depth_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	depth_dependency.dstSubpass = 0;
	depth_dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	depth_dependency.srcAccessMask = 0;
	depth_dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	depth_dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	const uint32_t attachment_count = 2;
	VkAttachmentDescription attachments[attachment_count] = { color_attachment, depth_attachment };

	const uint32_t dependency_count = 2;
	VkSubpassDependency dependencies[dependency_count] = { dependency, depth_dependency };

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.pNext = nullptr;
	render_pass_info.attachmentCount = attachment_count;
	render_pass_info.pAttachments = &attachments[0];
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = dependency_count;
	render_pass_info.pDependencies = &dependencies[0];

	VK_CHECK(vkCreateRenderPass(_device, &render_pass_info, nullptr, &_render_pass));

	_deletion_queue.push_function([=]() {
		vkDestroyRenderPass(_device, _render_pass, nullptr);
	});

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
		const uint32_t attachment_count = 2;
		VkImageView attachments[attachment_count] = { _swapchain_image_views[i], _depth_image_view };

		framebuffer_info.attachmentCount = attachment_count;
		framebuffer_info.pAttachments = &attachments[0];
		VK_CHECK(vkCreateFramebuffer(_device, &framebuffer_info, nullptr, &_framebuffers[i]));

		_deletion_queue.push_function([=]() {
			vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);
			vkDestroyImageView(_device, _swapchain_image_views[i], nullptr);
		});
	}

	return true;
}

bool Vulkun::_init_sync_structures() {
	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.pNext = nullptr;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VK_CHECK(vkCreateFence(_device, &fence_info, nullptr, &_render_fence));

	_deletion_queue.push_function([=]() {
		vkDestroyFence(_device, _render_fence, nullptr);
	});

	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_info.pNext = nullptr;

	VK_CHECK(vkCreateSemaphore(_device, &semaphore_info, nullptr, &_present_semaphore));
	VK_CHECK(vkCreateSemaphore(_device, &semaphore_info, nullptr, &_render_semaphore));

	_deletion_queue.push_function([=]() {
		vkDestroySemaphore(_device, _present_semaphore, nullptr);
		vkDestroySemaphore(_device, _render_semaphore, nullptr);
	});

	return true;
}

bool Vulkun::_init_imgui() {
	const uint32_t max_sets = 1000;
	VkDescriptorPoolSize pool_sizes[] = {
		{ VK_DESCRIPTOR_TYPE_SAMPLER, max_sets },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, max_sets },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, max_sets },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, max_sets },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, max_sets },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, max_sets },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, max_sets },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, max_sets },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, max_sets },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, max_sets },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, max_sets },
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.pNext = nullptr;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = max_sets;
	pool_info.poolSizeCount = std::size(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	VkDescriptorPool imgui_descriptor_pool;
	VK_CHECK(vkCreateDescriptorPool(_device, &pool_info, nullptr, &imgui_descriptor_pool));

	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForVulkan(_window);

	ImGui_ImplVulkan_InitInfo imgui_vulkan_init_info = {};
	imgui_vulkan_init_info.Instance = _instance;
	imgui_vulkan_init_info.PhysicalDevice = _physical_device;
	imgui_vulkan_init_info.Device = _device;
	imgui_vulkan_init_info.Queue = _graphics_queue;
	imgui_vulkan_init_info.DescriptorPool = imgui_descriptor_pool;
	imgui_vulkan_init_info.MinImageCount = 3;
	imgui_vulkan_init_info.ImageCount = 3;
	imgui_vulkan_init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	imgui_vulkan_init_info.RenderPass = _render_pass;

	ImGui_ImplVulkan_Init(&imgui_vulkan_init_info);

	// _immediate_submit([=](VkCommandBuffer cmd) {
	// 	ImGui_ImplVulkan_CreateFontsTexture();
	// });

	_deletion_queue.push_function([=]() {
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();

		vkDestroyDescriptorPool(_device, imgui_descriptor_pool, nullptr);
	});

	return true;
}

bool Vulkun::_init_pipelines() {
	bool success = false;

	VertexInputDescription mesh_vertex_input = Vertex::create_vertex_description();

	VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();

	VkPushConstantRange push_constant = {
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.offset = 0,
		.size = sizeof(PushConstants),
	};

	pipeline_layout_info.pushConstantRangeCount = 1;
	pipeline_layout_info.pPushConstantRanges = &push_constant;

	VkPipelineLayout pipeline_layout;
	VK_CHECK(vkCreatePipelineLayout(_device, &pipeline_layout_info, nullptr, &pipeline_layout));

	_deletion_queue.push_function([=]() {
		vkDestroyPipelineLayout(_device, pipeline_layout, nullptr);
	});

	VkShaderModule vert_shader_module, frag_shader_module;
	success = _load_shader_module(fmt::format("shaders/{}.vert.spv", "mesh_triangle").c_str(), &vert_shader_module);
	success = _load_shader_module(fmt::format("shaders/{}.frag.spv", "colored_triangle").c_str(), &frag_shader_module);

	if (!success) {
		fmt::println(stderr, "Failed to load shader modules for pipeline.");
		return false;
	}

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
	pipeline_builder.pipeline_layout = pipeline_layout;

	pipeline_builder.vertex_input_info.flags = mesh_vertex_input.flags;

	pipeline_builder.vertex_input_info.vertexBindingDescriptionCount = mesh_vertex_input.bindings.size();
	pipeline_builder.vertex_input_info.pVertexBindingDescriptions = mesh_vertex_input.bindings.data();
	fmt::println("Vertex binding description count: {}", pipeline_builder.vertex_input_info.vertexBindingDescriptionCount);

	pipeline_builder.vertex_input_info.vertexAttributeDescriptionCount = mesh_vertex_input.attributes.size();
	pipeline_builder.vertex_input_info.pVertexAttributeDescriptions = mesh_vertex_input.attributes.data();
	fmt::println("\tVertex attribute description count: {}", pipeline_builder.vertex_input_info.vertexAttributeDescriptionCount);

	pipeline_builder.depth_stencil = vkinit::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

	VkPipeline pipeline;
	pipeline = pipeline_builder.build_pipeline(_device, _render_pass);

	create_material(MaterialName::Default, pipeline, pipeline_layout);

	_deletion_queue.push_function([=]() {
		vkDestroyPipeline(_device, pipeline, nullptr);
	});

	return success;
}

bool Vulkun::_init_scene() {
	RenderObject monkey = {
		.pMesh = get_mesh(MeshName::Monkey),
		.pMaterial = get_material(MaterialName::Default),
		.transform = glm::translate(glm::mat4(1.0f), glm::vec3(0, 2, 0)),
		.update_push_constants = [=](PushConstants &push_constants) {
			push_constants.render_matrix *= glm::translate(glm::vec3(0, 0, 2 + sin(_frame_number * 0.02f) * 6));
			push_constants.render_matrix *= glm::rotate(sin(_frame_number * 0.03f) * 0.2f, glm::vec3(1, 0, 0));
		},
	};
	_renderables.push_back(monkey);

	for (int x = -20; x <= 20; ++x) {
		for (int y = -20; y <= 20; ++y) {
			glm::mat4 translation = glm::translate(glm::mat4{ 1.0f }, glm::vec3(x, -abs(y) * y * 0.1f, y));
			glm::mat4 scale = glm::scale(glm::mat4{ 1.0f }, glm::vec3(0.2f));
			RenderObject triangle = {
				.pMesh = get_mesh(MeshName::Triangle),
				.pMaterial = get_material(MaterialName::Default),
				.transform = translation * scale,
			};
			_renderables.push_back(triangle);
		}
	}

	return true;
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

	_deletion_queue.push_function([=]() {
		// TODO: It seems that the shader module can be destroyed immediately after pipeline creation.
		vkDestroyShaderModule(_device, shader_module, nullptr);
	});

	return true;
}

void Vulkun::_load_meshes() {
	Mesh triangle_mesh;
	triangle_mesh.vertices.resize(3);

	triangle_mesh.vertices[0].pos = { 1.0f, 1.0f, 0.0f };
	triangle_mesh.vertices[1].pos = { -1.0f, 1.0f, 0.0f };
	triangle_mesh.vertices[2].pos = { 0.0f, -1.0f, 0.0f };

	triangle_mesh.vertices[0].color = { 0.65f, 0.83f, 0.035f };
	triangle_mesh.vertices[1].color = { 0.83f, 0.65f, 0.035f };
	triangle_mesh.vertices[2].color = { 0.83f, 0.035f, 0.65f };

	_upload_mesh(triangle_mesh);
	_meshes[MeshName::Triangle] = triangle_mesh;

	Mesh monkey_mesh;
	monkey_mesh.load_from_obj("assets/opicka_smooth.obj");
	_upload_mesh(monkey_mesh);
	_meshes[MeshName::Monkey] = monkey_mesh;
}

void Vulkun::_upload_mesh(Mesh &mesh) {
	VkBufferCreateInfo buffer_info = {};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.pNext = nullptr;
	buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	buffer_info.size = mesh.size_of_vertices();

	fmt::println("Will upload mesh with size: {}", mesh.vertices.size());

	VmaAllocationCreateInfo vma_alloc_create_info = {};
	vma_alloc_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	VK_CHECK(vmaCreateBuffer(
			_allocator,
			&buffer_info,
			&vma_alloc_create_info,
			&mesh.vertex_buffer.buffer,
			&mesh.vertex_buffer.allocation,
			nullptr));

	_deletion_queue.push_function([=]() {
		vmaDestroyBuffer(_allocator, mesh.vertex_buffer.buffer, mesh.vertex_buffer.allocation);
	});

	void *vertex_data;
	vmaMapMemory(_allocator, mesh.vertex_buffer.allocation, &vertex_data);
	memcpy(vertex_data, mesh.vertices.data(), mesh.size_of_vertices());
	vmaUnmapMemory(_allocator, mesh.vertex_buffer.allocation);

	fmt::println("\tMesh uploaded");
}

void Vulkun::run() {
	bool should_quit = false;
	SDL_Event event;

	while (!should_quit) {
		float start_time = SDL_GetTicks();
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);

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
			}
		}

		if (should_quit) {
			break;
		}

		if (_is_rendering_paused) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();

		const uint8_t *keyboard_state = SDL_GetKeyboardState(nullptr);

		if (!ImGui::GetIO().WantCaptureKeyboard) {
			_camera.handle_input(keyboard_state);
		}

		_camera.update(_delta_time);

		draw();

		_delta_time = (SDL_GetTicks() - start_time) / 1000.0f;
	}
}

void Vulkun::_draw_objects(VkCommandBuffer command_buffer, RenderObject *pFirst_render_object, uint32_t count) {
	// fmt::println("Drawing {} objects", count);

	float aspect = (float)_window_extent.width / (float)_window_extent.height;

	Material *pLast_material = nullptr;
	Mesh *pLast_mesh = nullptr;
	for (uint32_t i = 0; i < count; ++i) {
		// fmt::println("\tDrawing object {}", i);
		RenderObject &render_object = pFirst_render_object[i];

		if (pLast_material != render_object.pMaterial) {
			vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_object.pMaterial->pipeline);
			pLast_material = render_object.pMaterial;
			// fmt::println("\t\tBound pipeline");
		}

		// fmt::println("\t\tObject transform: {}", glm::to_string(object.transform));
		glm::mat4 mesh_matrix = _camera.get_projection(aspect) * _camera.get_view() * render_object.transform;
		// fmt::println("\t\tModel matrix: {}", glm::to_string(mesh_matrix));

		PushConstants push_constants = {
			.render_matrix = mesh_matrix,
			.frame_number = _frame_number + i,
		};
		if (render_object.update_push_constants != nullptr) {
			render_object.update_push_constants(push_constants);
		}
		vkCmdPushConstants(command_buffer, render_object.pMaterial->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &push_constants);
		// fmt::println("\t\tPushed constants");

		if (pLast_mesh != render_object.pMesh) {
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(command_buffer, 0, 1, &render_object.pMesh->vertex_buffer.buffer, &offset);
			pLast_mesh = render_object.pMesh;
			// fmt::println("\t\tBound vertex buffer");
		}

		// TODO: Count draw calls
		vkCmdDraw(command_buffer, render_object.pMesh->vertices.size(), 1, 0, 0);
		// fmt::println("\t\tDrawn object");
	}

	// fmt::println("\tFinished drawning {} objects", count);
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

	VkClearValue clear_depth;
	clear_depth.depthStencil.depth = 1.0f;

	const uint32_t clear_value_count = 2;
	VkClearValue clear_values[clear_value_count] = { clear_color, clear_depth };

	VkRenderPassBeginInfo render_pass_begin_info = {};
	render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.pNext = nullptr;
	render_pass_begin_info.renderPass = _render_pass;
	render_pass_begin_info.renderArea.offset.x = 0;
	render_pass_begin_info.renderArea.offset.y = 0;
	render_pass_begin_info.renderArea.extent = _window_extent;
	render_pass_begin_info.framebuffer = _framebuffers[swapchain_image_index];
	render_pass_begin_info.clearValueCount = clear_value_count;
	render_pass_begin_info.pClearValues = &clear_values[0];

	vkCmdBeginRenderPass(_main_command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

	/**
	 * D R A W I N G
	 */

	_draw_objects(_main_command_buffer, _renderables.data(), _renderables.size());

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), _main_command_buffer);

	/**
	 * E N D   D R A W I N G
	 */

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

	_deletion_queue.flush();
}

Material *Vulkun::create_material(const std::string &name, VkPipeline pipeline, VkPipelineLayout pipeline_layout) {
	Material material = {
		.pipeline = pipeline,
		.pipeline_layout = pipeline_layout,
	};
	_materials[name] = material;

	return &_materials[name];
}

Material *Vulkun::get_material(const std::string &name) {
	auto it = _materials.find(name);
	if (it == _materials.end()) {
		fmt::println(stderr, "Material with name {} not found", name);
		abort();
	}
	return &it->second;
}

Mesh *Vulkun::get_mesh(const std::string &name) {
	auto it = _meshes.find(name);
	if (it == _meshes.end()) {
		fmt::println(stderr, "Mesh with name {} not found", name);
		abort();
	}
	return &it->second;
}
