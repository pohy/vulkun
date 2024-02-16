#include "camera.h"

void Camera::handle_input(const uint8_t *keyboard_state, Mouse &mouse) {
	_handle_keyboard(keyboard_state);

	if (mouse.right) {
		_rot_amount = mouse.delta;
	}
	if (mouse.right_pressed) {
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}
	if (mouse.right_released) {
		_rot_amount = glm::vec2(0);
		SDL_SetRelativeMouseMode(SDL_FALSE);
	}
}

glm::mat4 Camera::get_projection(float aspect) {
	auto projection = glm::perspective(glm::radians(fov), aspect, near, far);
	projection[1][1] *= -1;
	return projection;
}

void Camera::_handle_keyboard(const uint8_t *keyboard_state) {
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

void Camera::update(float delta_time) {
	// https://community.khronos.org/t/get-direction-from-transformation-matrix-or-quat/65502/2
	glm::vec3 right = glm::inverse(_transform)[0];
	glm::vec3 up = glm::inverse(_transform)[1];
	glm::vec3 forward = glm::inverse(_transform)[2];

	if (glm::length(_move_dir) > 0.0f) {
		_move_dir = glm::normalize(_move_dir);
	}

	if (glm::length(_rot_amount) > 0.0f) {
		_transform = glm::rotate(_transform, _rot_amount.y * _mouse_sens * delta_time, right);
		_transform = glm::rotate(_transform, _rot_amount.x * _mouse_sens * delta_time, glm::vec3(0, 1, 0));
	}

	if (_is_sprinting) {
		_move_dir *= _sprint_mult;
	}

	_transform = glm::translate(_transform, _move_dir * _speed * delta_time);
}
