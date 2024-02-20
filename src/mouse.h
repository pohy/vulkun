#pragma once

#include <SDL2/SDL.h>
#include <fmt/core.h>
#include <glm/glm.hpp>

struct Mouse {
	glm::ivec2 pos{0};
	glm::ivec2 delta{0};

	bool left = false;
	bool left_pressed = false;
	bool left_released = false;

	bool right = false;
	bool right_pressed = false;
	bool right_released = false;

	void update(float delta_time) {
		SDL_GetRelativeMouseState(&delta.x, &delta.y);

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
