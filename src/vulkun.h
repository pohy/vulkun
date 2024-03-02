#pragma once

#include "camera.h"
#include "deletion_queue.h"
#include "game_objects.h"
#include "material.h"
#include "mesh.h"
#include "mouse.h"
#include "vk_types.h"

#include <glm/glm.hpp>

#define APP_NAME "Vulkun - ゔるくん"

struct IGameObject;

struct PushConstants {
	glm::mat4 render_matrix;
	uint32_t frame_number;
};

struct Metrics {
	uint32_t draw_calls = 0;
	uint32_t pipeline_bind_calls = 0;
	uint32_t vertex_buffer_bind_calls = 0;

	void reset() {
		draw_calls = 0;
		pipeline_bind_calls = 0;
		vertex_buffer_bind_calls = 0;
	}
};

class Vulkun {
private:
	uint32_t _frame_number = 0;
	float _delta_time = 0.0f;
	int _selected_pipeline_idx = 0;
	bool _is_initialized = false;
	bool _is_rendering_paused = false;
	Metrics _metrics;

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

	VkImageView _depth_image_view;
	AllocatedImage _depth_image;
	VkFormat _depth_format = VK_FORMAT_D32_SFLOAT;

	VkQueue _graphics_queue;
	uint32_t _graphics_queue_family_idx;
	VkCommandPool _command_pool;
	VkCommandBuffer _main_command_buffer;

	VkRenderPass _render_pass;
	std::vector<VkFramebuffer> _framebuffers;

	VkSemaphore _present_semaphore, _render_semaphore;
	VkFence _render_fence;

	std::vector<IGameObject *> _game_objects;
	std::unordered_map<std::string, Material> _materials;
	std::unordered_map<std::string, Mesh> _meshes;

	Mouse _mouse;
	Camera _camera;

	bool _init_vulkan();
	bool _init_swapchain();
	bool _init_commands();
	bool _init_default_renderpass();
	bool _init_framebuffers();
	bool _init_sync_structures();
	bool _init_pipelines();
	bool _init_scene();
	bool _init_imgui();

	bool _load_shader_module(const char *file_path, VkShaderModule *out_shader_module);
	void _load_meshes();
	void _upload_mesh(Mesh &mesh);
	VkPipeline _create_vert_frag_pipeline(const std::string &vert_name, const std::string &frag_name, VkPipelineLayout &pipeline_layout);

	// TODO: Either remove the pointer passing or figure why the material of triangles points to a nullptr
	void _draw_objects(VkCommandBuffer command_buffer, IGameObject *pFirst_game_object, uint32_t count);

public:
	static Vulkun &get_singleton();

	void init();
	void run();
	void draw();
	void cleanup();

	bool is_initialized() const { return _is_initialized; }

	uint32_t frame_number() const { return _frame_number; }

	Material *create_material(const std::string &name, const std::string &vert_name, const std::string &frag_name, VkPipelineLayout pipeline_layout);
	Material *create_material(const std::string &name, VkPipeline pipeline, VkPipelineLayout pipeline_layout);
	Material *get_material(const std::string &name);
	Mesh *get_mesh(const std::string &name);
};
