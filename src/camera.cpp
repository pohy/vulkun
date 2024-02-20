#include "camera.h"

#include <imgui.h>

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

glm::mat4 Camera::get_view() {
	return glm::lookAt(_pos, _pos + _forward, _up);
}

glm::mat4 Camera::get_projection(float aspect) {
	return glm::perspective(glm::radians(fov), aspect, near, far);
}

void Camera::update(float delta_time) {
	ImGui::Begin("Camera");
	ImGui::SliderFloat("FOV", &fov, 1.0f, 180.0f);
	ImGui::SliderFloat("Sensitivity", &_mouse_sens, 1.0f, 10.0f);
	ImGui::SliderFloat("Speed", &_speed, 1.0f, 100.0f);
	ImGui::SliderFloat("Sprint Multiplier", &_sprint_mult, 1.0f, 10.0f);
	ImGui::Text("Position: (%.2f, %.2f, %.2f)", _pos.x, _pos.y, _pos.z);
	ImGui::Text("Yaw: %.2f", _yaw);
	ImGui::Text("Pitch: %.2f", _pitch);
	ImGui::Text("Forward: (%.2f, %.2f, %.2f)", _forward.x, _forward.y, _forward.z);
	ImGui::Text("Right: (%.2f, %.2f, %.2f)", _right.x, _right.y, _right.z);
	ImGui::Text("Up: (%.2f, %.2f, %.2f)", _up.x, _up.y, _up.z);
	ImGui::End();

	_yaw += _rot_amount.x * _mouse_sens * delta_time;
	_pitch += _rot_amount.y * _mouse_sens * delta_time;

	glm::vec3 direction{
		cos(glm::radians(_yaw)) * cos(glm::radians(_pitch)),
		sin(glm::radians(_pitch)),
		sin(glm::radians(_yaw)) * cos(glm::radians(_pitch))
	};
	_forward = glm::normalize(direction);
	_right = glm::normalize(glm::cross(_forward, _up));

	if (glm::length(_move_input) > 0.0f) {
		_move_input = glm::normalize(_move_input);
	}

	if (_is_sprinting) {
		_move_input *= _sprint_mult;
	}

	glm::vec3 pos_delta = _forward * _move_input.z + _right * _move_input.x + _up * _move_input.y;
	_pos += pos_delta * _speed * delta_time;
}

void Camera::_handle_keyboard(const uint8_t *keyboard_state) {
	// TODO: _move_input would be a more appropriate name
	_move_input = glm::vec3(0);
	if (keyboard_state[SDL_SCANCODE_W] || keyboard_state[SDL_SCANCODE_UP]) {
		_move_input.z += 1;
	}
	if (keyboard_state[SDL_SCANCODE_S] || keyboard_state[SDL_SCANCODE_DOWN]) {
		_move_input.z -= 1;
	}
	if (keyboard_state[SDL_SCANCODE_A] || keyboard_state[SDL_SCANCODE_LEFT]) {
		_move_input.x -= 1;
	}
	if (keyboard_state[SDL_SCANCODE_D] || keyboard_state[SDL_SCANCODE_RIGHT]) {
		_move_input.x += 1;
	}
	if (keyboard_state[SDL_SCANCODE_SPACE]) {
		_move_input.y -= 1;
	}
	if (keyboard_state[SDL_SCANCODE_C]) {
		_move_input.y += 1;
	}

	_is_sprinting = keyboard_state[SDL_SCANCODE_LSHIFT];
}
