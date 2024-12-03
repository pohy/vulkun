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
	glm::mat4 model_matrix;
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

struct GPUCameraData {
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 view_proj;
};

struct GPUSceneData {
	glm::vec4 sun_direction{ -1.0f, -1.0f, -1.0f, 0.0f };
	glm::vec4 sun_color{ 1.0f, 1.0f, 1.0f, 1.0f };
	glm::vec4 ambient_color{ 1.0f, 1.0f, 1.0f, 0.2f }; // w for intensity
	glm::vec4 fog_color; // w for exponent
	glm::vec4 fog_distances; // x for start, y for end, zw unused
};

struct FrameData {
	VkSemaphore present_semaphore;
	VkSemaphore render_semaphore;
	VkFence render_fence;
	VkCommandPool command_pool;
	VkCommandBuffer command_buffer;
	VkDescriptorSet global_descriptors;

	AllocatedBuffer camera_data_buffer;
};

constexpr uint32_t FRAME_OVERLAP = 2;

class Vulkun {
private:
	uint32_t _frame_number = 0;
	float _delta_time = 0.0f;
	int _selected_pipeline_idx = 0;
	bool _is_initialized = false;
	bool _is_rendering_paused = false;

	Metrics _metrics;
	FrameData _frame_data[FRAME_OVERLAP];

	GPUSceneData _scene_data;
	AllocatedBuffer _scene_data_buffer; // Date for all FRAME_OVERLAP be stored in the single buffer

	VkExtent2D _window_extent = { 1600, 900 };
	struct std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> _window;

	DeletionQueue _deletion_queue;

	VkInstance _instance;
	VkDebugUtilsMessengerEXT _debug_messenger;
	VkPhysicalDevice _physical_device;
	VkPhysicalDeviceProperties _physical_device_properties;
	VkDevice _device;
	VkSurfaceKHR _surface;

	VkDescriptorSetLayout _global_descriptors_layout;
	VkDescriptorPool _descriptor_pool;

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

	VkRenderPass _render_pass;
	std::vector<VkFramebuffer> _framebuffers;

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
	bool _init_descriptors();

	bool _load_shader_module(const char *file_path, VkShaderModule *out_shader_module);
	void _load_meshes();
	void _upload_mesh(Mesh &mesh);
	VkPipeline _create_vert_frag_pipeline(const std::string &vert_name, const std::string &frag_name, VkPipelineLayout &pipeline_layout);
	AllocatedBuffer _create_buffer(size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage);
	size_t _pad_uniform_buffer_size(size_t original_size);

	// TODO: Either remove the pointer passing or figure why the material of triangles points to a nullptr
	void _draw_objects(VkCommandBuffer command_buffer);

	FrameData &_get_current_frame_data();

public:
	Vulkun() :
			_window(nullptr, SDL_DestroyWindow) {}

	void init();
	void run();
	void draw();
	void cleanup();

	bool is_initialized() const { return _is_initialized; }

	uint32_t frame_number() const { return _frame_number; }

	Material *create_material(const std::string &name, const std::string &vert_name, const std::string &frag_name, VkPipelineLayout pipeline_layout, const VkDescriptorSetLayout *pDescriptor_layout);
	Material *create_material(const std::string &name, VkPipeline pipeline, VkPipelineLayout pipeline_layout, const VkDescriptorSetLayout *pDescriptor_layout);
	Material *get_material(const std::string &name);
	Mesh *get_mesh(const std::string &name);
};
