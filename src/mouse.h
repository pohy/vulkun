#pragma once

#include <SDL2/SDL.h>
#include <fmt/core.h>
#include <glm/glm.hpp>

class Mouse {
private:
	bool _prev_has_focus = false;

	void _update_delta(glm::ivec2 new_pos) {
		bool has_focus = SDL_GetMouseFocus() != nullptr;

		// Prevent jumping when the cursor returns to the window at a different position
		if (_prev_has_focus && has_focus) {
			delta = new_pos - pos;
		} else {
			delta.x = 0;
			delta.y = 0;
		}

		_prev_has_focus = has_focus;
	}

public:
	glm::ivec2 pos;
	glm::ivec2 delta;

	bool left = false;
	bool left_pressed = false;
	bool left_released = false;

	bool right = false;
	bool right_pressed = false;
	bool right_released = false;

	void update(float delta_time) {
		glm::ivec2 new_pos;
		uint32_t buttons = SDL_GetMouseState(&new_pos.x, &new_pos.y);

		_update_delta(new_pos);
		pos = new_pos;

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
