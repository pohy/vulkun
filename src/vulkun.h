#pragma once

#include "deletion_queue.h"
#include "vk_mesh.h"
#include "vk_types.h"

#define APP_NAME "Vulkun - ゔるくん"

struct PipelineStagesInfo {
	std::string vertex_name;
	std::string fragment_name;

	// VertexInputDescription vertex_input;
	VertexInputDescription *pVertex_input = nullptr;

	// ~PipelineStagesInfo() {
	// 	fmt::println("PipelineStagesInfo destructor");
	// 	// if (pVertex_input) {
	// 	// 	fmt::println("Deleting pVertex_input");
	// 	// 	delete pVertex_input;
	// 	// }
	// }
};

class Vulkun {
private:
	int _frame_number = 0;
	int _selected_pipeline_idx = 0;
	bool _is_initialized = false;
	bool _is_rendering_paused = false;

	VkExtent2D _window_extent = { 1600, 900 };
	struct SDL_Window *_window = nullptr;

	DeletionQueue _deletion_queue;

	VkInstance _instance;
	VkDebugUtilsMessengerEXT _debug_messenger;
	VkPhysicalDevice _physical_device;
	VkDevice _device;
	VkSurfaceKHR _surface;

	VmaAllocator _allocator;

	VkSwapchainKHR _swapchain;
	VkFormat _swapchain_image_format;
	std::vector<VkImage> _swapchain_images;
	std::vector<VkImageView> _swapchain_image_views;

	VkQueue _graphics_queue;
	uint32_t _graphics_queue_family_idx;
	VkCommandPool _command_pool;
	VkCommandBuffer _main_command_buffer;

	VkRenderPass _render_pass;
	std::vector<VkFramebuffer> _framebuffers;

	VkSemaphore _present_semaphore, _render_semaphore;
	VkFence _render_fence;

	VkPipelineLayout _pipeline_layout;
	std::vector<VkPipeline> _pipelines;
	Mesh _triangle_mesh;

	std::vector<PipelineStagesInfo> _stages_info;

	bool _init_vulkan();
	bool _init_swapchain();
	bool _init_commands();
	bool _init_default_renderpass();
	bool _init_framebuffers();
	bool _init_sync_structures();
	bool _init_pipelines();

	bool _load_shader_module(const char *file_path, VkShaderModule *out_shader_module);
	void _load_meshes();
	void _upload_mesh(Mesh &mesh);

public:
	static Vulkun &get_singleton();

	void init();
	void run();
	void draw();
	void cleanup();

	bool is_initialized() const { return _is_initialized; }
};
