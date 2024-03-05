#pragma once

#include <SDL2/SDL.h>
#include <fmt/core.h>
#include <glm/glm.hpp>

class Mouse {
private:
	glm::vec2 _screen_size{ 0 };

	void _update_delta() {
		int x, y;
		SDL_GetRelativeMouseState(&x, &y);
		if (glm::length(_screen_size) > 0.0f) {
			// Normalize delta
			delta = glm::vec2(x, y) / _screen_size;
		}
	}

public:
	glm::ivec2 pos{ 0 };
	glm::vec2 delta{ 0 };

	bool left = false;
	bool left_pressed = false;
	bool left_released = false;

	bool right = false;
	bool right_pressed = false;
	bool right_released = false;

	void on_window_moved(uint32_t window_index) {
		SDL_DisplayMode display_mode;
		SDL_GetCurrentDisplayMode(window_index, &display_mode);
		_screen_size = glm::vec2(display_mode.w, display_mode.h);
	}

	void update(float delta_time) {
		_update_delta();

		uint32_t buttons = SDL_GetMouseState(&pos.x, &pos.y);

		bool prev_left = left;
		// TODO: Figure out why we use bitwise AND here instead of a logical AND
		left = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
		left_pressed = !prev_left && left;
		left_released = prev_left && !left;

		bool prev_right = right;
		right = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);
		right_pressed = !prev_right && right;
		right_released = prev_right && !right;
	}
};
