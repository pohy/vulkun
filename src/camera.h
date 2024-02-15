#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <SDL2/SDL.h>
#include <fmt/core.h>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

class Camera {
private:
	glm::vec3 _move_dir;
	float _speed = 10.0f;
	float _sprint_mult = 2.0f;
	bool _is_sprinting = false;

public:
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(0, -2, -10));
	;
	float fov = 70.0f;
	float near = 0.1f;
	float far = 200.0f;

	void handle_input(const uint8_t *keyboard_state) {
		_move_dir = glm::vec3(0);
		if (keyboard_state[SDL_SCANCODE_W] || keyboard_state[SDL_SCANCODE_UP]) {
			_move_dir.z += 1;
		}
		if (keyboard_state[SDL_SCANCODE_S] || keyboard_state[SDL_SCANCODE_DOWN]) {
			_move_dir.z -= 1;
		}
		if (keyboard_state[SDL_SCANCODE_A] || keyboard_state[SDL_SCANCODE_LEFT]) {
			_move_dir.x += 1;
		}
		if (keyboard_state[SDL_SCANCODE_D] || keyboard_state[SDL_SCANCODE_RIGHT]) {
			_move_dir.x -= 1;
		}
		if (keyboard_state[SDL_SCANCODE_SPACE]) {
			_move_dir.y -= 1;
		}
		if (keyboard_state[SDL_SCANCODE_C]) {
			_move_dir.y += 1;
		}

		_is_sprinting = keyboard_state[SDL_SCANCODE_LSHIFT];
	}

	void update(float delta_time) {
		if (glm::length(_move_dir) > 0.0f) {
			_move_dir = glm::normalize(_move_dir);
		}
		if (_is_sprinting) {
			_move_dir *= _sprint_mult;
		}
		transform = glm::translate(transform, _move_dir * _speed * delta_time);
	}

	glm::mat4 get_projection(float aspect) {
		auto projection = glm::perspective(glm::radians(fov), aspect, near, far);
		projection[1][1] *= -1;
		return projection;
	}

	glm::mat4 get_view() {
		return transform;
	}

	glm::vec3 get_pos() {
		return glm::vec3(transform[3]);
	}
};
