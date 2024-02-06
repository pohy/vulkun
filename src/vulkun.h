#pragma once

#include "vk_types.h"

class Vulkun {
  private:
	int _frame_number = 0;
	bool _is_initialized = false;
	bool _is_rendering_paused = false;

	VkExtent2D _window_extent = {1920, 1080};
	struct SDL_Window *_window = nullptr;

	VkInstance _instance;
	VkDebugUtilsMessengerEXT _debug_messenger;
	VkPhysicalDevice _physical_device;
	VkDevice _device;
	VkSurfaceKHR _surface;

	VkSwapchainKHR _swapchain;
	VkFormat _swapchain_image_format;
	std::vector<VkImage> _swapchain_images;
	std::vector<VkImageView> _swapchain_image_views;

	VkQueue _graphics_queue;
	uint32_t _graphics_queue_family_idx;
	VkCommandPool _command_pool;
	VkCommandBuffer _main_command_buffer;

	bool _init_vulkan();
	bool _init_swapchain();
	bool _init_commands();
  public:
	static Vulkun &get_singleton();

	void init();
	void run();
	void draw();
	void cleanup();

	bool is_initialized() const { return _is_initialized; }
};
