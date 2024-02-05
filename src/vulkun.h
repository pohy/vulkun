#pragma once

#include "vk_types.h"

class Vulkun {
  private:
	int _frame_number = 0;
	bool _is_initialized = false;
	bool _is_rendering_paused = false;

	VkExtent2D _window_extent = {1920, 1080};
	struct SDL_Window *_window = nullptr;

  public:
	static Vulkun &get_singleton();

	void init();
	void run();
	void draw();
	void cleanup();
};
