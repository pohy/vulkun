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

	_deletion_queue.push_function([=, this]() {
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

	_deletion_queue.push_function([=, this]() {
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

	_deletion_queue.push_function([=, this]() {
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

	_deletion_queue.push_function([=, this]() {
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

	_deletion_queue.push_function([=, this]() {
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

	_deletion_queue.push_function([=, this]() {
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

	_deletion_queue.push_function([=, this]() {
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

	_deletion_queue.push_function([=, this]() {
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

		_deletion_queue.push_function([=, this]() {
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

	_deletion_queue.push_function([=, this]() {
		vkDestroyFence(_device, _render_fence, nullptr);
	});

	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_info.pNext = nullptr;

	VK_CHECK(vkCreateSemaphore(_device, &semaphore_info, nullptr, &_present_semaphore));
	VK_CHECK(vkCreateSemaphore(_device, &semaphore_info, nullptr, &_render_semaphore));

	_deletion_queue.push_function([=, this]() {
		vkDestroySemaphore(_device, _present_semaphore, nullptr);
		vkDestroySemaphore(_device, _render_semaphore, nullptr);
	});

	return true;
}

bool Vulkun::_init_imgui() {
	const uint32_t max_sets = 1; //000;
	// We'll need more dscriptor sets for textures and stuff
	VkDescriptorPoolSize pool_sizes[] = {
		// { VK_DESCRIPTOR_TYPE_SAMPLER, max_sets },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, max_sets },
		// { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, max_sets },
		// { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, max_sets },
		// { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, max_sets },
		// { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, max_sets },
		// { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, max_sets },
		// { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, max_sets },
		// { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, max_sets },
		// { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, max_sets },
		// { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, max_sets },
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

	_deletion_queue.push_function([=, this]() {
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();

		vkDestroyDescriptorPool(_device, imgui_descriptor_pool, nullptr);
	});

	return true;
}

bool Vulkun::_init_pipelines() {
	VkPushConstantRange push_constant = {
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.offset = 0,
		.size = sizeof(PushConstants),
	};

	VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();
	pipeline_layout_info.pushConstantRangeCount = 1;
	pipeline_layout_info.pPushConstantRanges = &push_constant;

	VkPipelineLayout pipeline_layout;
	VK_CHECK(vkCreatePipelineLayout(_device, &pipeline_layout_info, nullptr, &pipeline_layout));

	_deletion_queue.push_function([=, this]() {
		vkDestroyPipelineLayout(_device, pipeline_layout, nullptr);
	});

	// D E F A U L T   M A T E R I A L
	create_material(MaterialName::Default, "mesh_triangle", "colored_triangle", pipeline_layout);

	// S H I F T I N G   C O L O R S   M A T E R I A L
	create_material(MaterialName::ShiftingColors, "mesh_triangle", "shifting_colors", pipeline_layout);

	// I M P R E Z A    M A T E R I A L
	create_material(MaterialName::Impreza, "mesh_triangle", "impreza", pipeline_layout);

	return true;
}

VkPipeline Vulkun::_create_vert_frag_pipeline(const std::string &vert_name, const std::string &frag_name, VkPipelineLayout &pipeline_layout) {
	VkShaderModule vert_shader_module, frag_shader_module;
	bool success = _load_shader_module(fmt::format("shaders/{}.vert.spv", vert_name).c_str(), &vert_shader_module);
	success = _load_shader_module(fmt::format("shaders/{}.frag.spv", frag_name).c_str(), &frag_shader_module);

	if (!success) {
		fmt::println(stderr, "Failed to load shader modules for pipeline.");
		abort();
	}

	PipelineBuilder pipeline_builder = PipelineBuilder::create_vert_frag_pipeline(vert_shader_module, frag_shader_module, _window_extent);
	pipeline_builder.pipeline_layout = pipeline_layout;

	VkPipeline pipeline = pipeline_builder.build_pipeline(_device, _render_pass);

	_deletion_queue.push_function([=, this]() {
		vkDestroyPipeline(_device, pipeline, nullptr);
	});

	return pipeline;
}

bool Vulkun::_init_scene() {
	IGameObject *pKing_ape = new Monkey(*this, 0);
	pKing_ape->transform.set_scale(1.5f);
	pKing_ape->transform.translate(glm::vec3{ 0, 2, 0 });
	_game_objects.push_back(pKing_ape);

	float radius = 5.0f;
	size_t count = 10;
	float frequency = 2 * M_PI / count;

	for (uint32_t i = 0; i < count; ++i) {
		IGameObject *pMonkey = new Monkey(*this, 1 + i);

		glm::vec3 pos = glm::vec3{ radius * cos(i * frequency), 0, radius * sin(i * frequency) };
		pMonkey->transform.translate(pos);
		pMonkey->transform.look_at(pKing_ape->transform.pos());

		_game_objects.push_back(pMonkey);
	}

	pKing_ape->transform.look_at(_game_objects[1]->transform.pos());

	for (int x = -20; x <= 20; ++x) {
		for (int y = -20; y <= 20; ++y) {
			IGameObject *pTriangle = new Triangle(*this);
			pTriangle->transform.translate(glm::vec3{ x, -4 + abs(y) * y * 0.1f, -y });
			pTriangle->transform.set_scale(glm::vec3{ 0.3f });

			_game_objects.push_back(pTriangle);
		}
	}

	IGameObject *pImpreza = new Impreza(*this);
	pImpreza->transform.translate(glm::vec3{ 0, -2, 0 });
	_game_objects.push_back(pImpreza);

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

	_deletion_queue.push_function([=, this]() {
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

	Mesh impreza_mesh;
	impreza_mesh.load_from_obj("assets/impreza/RR22B.obj");
	_upload_mesh(impreza_mesh);
	_meshes[MeshName::Impreza] = impreza_mesh;
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

	_deletion_queue.push_function([=, this]() {
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
				if (event.window.event == SDL_WINDOW_MINIMIZED || event.window.event == SDL_WINDOW_HIDDEN) {
					// TODO: Pause rendering when the window is not focused
					_is_rendering_paused = true;
				}
				if (event.window.event == SDL_WINDOW_MAXIMIZED || event.window.event == SDL_WINDOW_SHOWN) {
					_is_rendering_paused = false;
				}
			}

			if (event.type == SDL_KEYUP) {
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					should_quit = true;
					break;
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

		static bool show_demo = false;
		ImGui::BeginMainMenuBar();
		if (ImGui::BeginMenu("Toggle demo")) {
			show_demo = !show_demo;
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();

		if (show_demo) {
			ImGui::ShowDemoWindow(&show_demo);
		}

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
		ImGui::SetNextWindowBgAlpha(0.35f);
		const uint32_t pad = 10;
		ImGui::SetNextWindowPos(ImVec2(pad, _window_extent.height - pad), ImGuiCond_Always, ImVec2(0, 1));

		ImGui::Begin("Vulkun", nullptr, window_flags);

		ImGui::Text("Frame time: %.0fms", _delta_time * 1000.0f);

		ImGui::Text("Draw calls: %d", _draw_calls);

		glm::vec3 camera_pos = _camera.get_pos();
		ImGui::Text("Camera pos: %.2f, %.2f, %.2f", camera_pos.x, camera_pos.y, camera_pos.z);

		ImGui::Text("Mouse pos: %d, %d", _mouse.pos.x, _mouse.pos.y);
		ImGui::Text("Mouse delta: %d, %d", _mouse.delta.x, _mouse.delta.y);
		ImGui::Text("Mouse left: %d, right: %d", _mouse.left, _mouse.right);

		ImGui::End();

		const uint8_t *keyboard_state = SDL_GetKeyboardState(nullptr);

		if (!ImGui::GetIO().WantCaptureKeyboard) {
			_camera.handle_input(keyboard_state, _mouse);
		}

		_mouse.update(_delta_time);
		_camera.update(_delta_time);

		for (auto &game_object : _game_objects) {
			game_object->update(_delta_time);
		}

		draw();

		_delta_time = (SDL_GetTicks() - start_time) / 1000.0f;
	}
}

// TODO: Figure out why the "pointer" approach ends up pointing to a nullptr when accessing the render object's material
void Vulkun::_draw_objects(VkCommandBuffer command_buffer, IGameObject *_, uint32_t __) {
	// fmt::println("Drawing {} objects", count);
	_draw_calls = 0;

	float aspect = (float)_window_extent.width / (float)_window_extent.height;
	glm::mat4 projection = _camera.get_projection(aspect);
	glm::mat4 view = _camera.get_view();

	Material *pLast_material = nullptr;
	Mesh *pLast_mesh = nullptr;

	for (uint32_t i = 0; i < _game_objects.size(); ++i) {
		// fmt::println("\tDrawing object {}", i);
		IGameObject &game_object = *_game_objects[i];
		RenderObject &render_object = game_object.render_object;

		if (pLast_material != render_object.pMaterial) {
			ASSERT_MSG(render_object.pMaterial != nullptr, "Render object has no material");
			ASSERT_MSG(render_object.pMaterial->pipeline != VK_NULL_HANDLE, "Material has no pipeline");

			vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_object.pMaterial->pipeline);
			pLast_material = render_object.pMaterial;
			// fmt::println("\t\tBound pipeline");
		}

		// fmt::println("\t\tObject transform: {}", glm::to_string(object.transform));

		// Mesh matrix = Model View Projection matrix
		glm::mat4 mesh_matrix = projection * view * game_object.transform.get_model();
		// fmt::println("\t\tModel matrix: {}", glm::to_string(mesh_matrix));

		PushConstants push_constants = {
			.render_matrix = mesh_matrix,
			.frame_number = _frame_number + i,
		};

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
		_draw_calls++;
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
	float flash = abs(sin(_frame_number / 360.0f));
	clear_color.color = { { 1.0f - flash, flash, flash * 0.5f, 1.0f } };
	for (size_t i = 0; i < 3; i++) {
		clear_color.color.float32[i] *= 0.2f;
	}

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

	_draw_objects(_main_command_buffer, *_game_objects.data(), _game_objects.size());

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

Material *Vulkun::create_material(const std::string &name, const std::string &vert_name, const std::string &frag_name, VkPipelineLayout pipeline_layout) {
	// TODO: How about inlining the _create_vert_frag_pipeline method?
	VkPipeline pipeline = _create_vert_frag_pipeline(vert_name, frag_name, pipeline_layout);
	return create_material(name, pipeline, pipeline_layout);
}

Material *Vulkun::create_material(const std::string &name, VkPipeline pipeline, VkPipelineLayout pipeline_layout) {
	Material material = {
		.pipeline = pipeline,
		.pipeline_layout = pipeline_layout,
	};
	_materials[name] = material;

	fmt::println("Created material with name: '{}'", name);

	return &_materials[name];
}

Material *Vulkun::get_material(const std::string &name) {
	auto it = _materials.find(name);
	if (it == _materials.end()) {
		fmt::println(stderr, "Material with name '{}' not found", name);
		abort();
	}
	return &it->second;
}

Mesh *Vulkun::get_mesh(const std::string &name) {
	auto it = _meshes.find(name);
	if (it == _meshes.end()) {
		fmt::println(stderr, "Mesh with name '{}' not found", name);
		abort();
	}
	return &it->second;
}
