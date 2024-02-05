#include "vulkun.h"
#include "vk_types.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <cassert>
#include <chrono>
#include <thread>

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

	_is_initialized = true;
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
		SDL_DestroyWindow(_window);
	}

	singleton_instance = nullptr;
}
