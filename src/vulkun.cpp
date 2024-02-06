#include "vulkun.h"
#include "vk_types.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#define VK_KHR_portability_enumeration 1
#include <VkBootstrap.h>

#include <cassert>
#include <chrono>
#include <iostream>
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

	_window = SDL_CreateWindow("Vulkun", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _window_extent.width, _window_extent.height, window_flags);

	_is_initialized = _init_vulkan();
	_is_initialized = _init_swapchain();
}

bool Vulkun::_init_vulkan() {
	vkb::InstanceBuilder builder;

	auto inst_ret = builder
		.set_app_name("Vulkun")
		.require_api_version(1, 2, 0)
		.request_validation_layers(enable_validation_layers)
		.use_default_debug_messenger()
		.build();

	if (!inst_ret) {
		std::cerr << "Failed to create Vulkan instance. Error: " << inst_ret.error().message() << "\n";
		std::cerr << "Result: " << inst_ret.vk_result() << "\n";
		return false;
	}

	vkb::Instance vkb_inst = inst_ret.value();

	_instance = vkb_inst.instance;
	_debug_messenger = vkb_inst.debug_messenger;

	SDL_Vulkan_CreateSurface(_window, _instance, &_surface);

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
void Vulkun::run() {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	SDL_Window *window = SDL_CreateWindow("Vulkun", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, 0);

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
			}
		}

		if (_is_rendering_paused) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		draw();
	}
}

void Vulkun::draw() {}

void Vulkun::cleanup() {
	if (_is_initialized) {
		vkDestroySwapchainKHR(_device, _swapchain, nullptr);
		for (auto &image_view : _swapchain_image_views) {
			vkDestroyImageView(_device, image_view, nullptr);
		}
		vkDestroyDevice(_device, nullptr);
		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
		vkDestroyInstance(_instance, nullptr);

		SDL_DestroyWindow(_window);
	}

	singleton_instance = nullptr;
}
