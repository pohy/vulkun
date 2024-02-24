#include "camera.h"

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>

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
	return glm::lookAt(transform.pos(), transform.pos() + transform.forward(), transform.up());
}

glm::mat4 Camera::get_projection(float aspect) {
	glm::mat4 projection = glm::perspective(glm::radians(fov), aspect, near, far);
	projection[1][1] *= -1;
	return projection;
}

void Camera::update(float delta_time) {
	ImGui::Begin("Camera");
	ImGui::SliderFloat("FOV", &fov, 1.0f, 180.0f);
	ImGui::SliderFloat("Sensitivity", &_mouse_sens, 1.0f, 10.0f);
	ImGui::SliderFloat("Speed", &_speed, 1.0f, 100.0f);
	ImGui::SliderFloat("Sprint Multiplier", &_sprint_mult, 1.0f, 10.0f);
	ImGui::Text("Position: (%.2f, %.2f, %.2f)", transform.pos().x, transform.pos().y, transform.pos().z);
	ImGui::Text("Yaw: %.2f", transform.rot().y);
	ImGui::Text("Pitch: %.2f", transform.rot().x);
	ImGui::Text("Forward: (%.2f, %.2f, %.2f)", transform.forward().x, transform.forward().y, transform.forward().z);
	ImGui::Text("Right: (%.2f, %.2f, %.2f)", transform.right().x, transform.right().y, transform.right().z);
	ImGui::Text("Up: (%.2f, %.2f, %.2f)", transform.up().x, transform.up().y, transform.up().z);
	ImGui::End();

	transform.rotate(-_rot_amount.x * _mouse_sens * delta_time, glm::vec3(0, 1, 0));
	transform.rotate(-_rot_amount.y * _mouse_sens * delta_time, transform.right());

	glm::vec3 rot = transform.rot();
	transform.set_rot(glm::vec3(glm::clamp(rot.x, _vertical_clamp.x, _vertical_clamp.y), rot.y, rot.z));
	transform.set_up(glm::vec3(0, 1, 0));

	if (glm::length(_move_input) > 0.0f) {
		_move_input = glm::normalize(_move_input);
	}

	if (_is_sprinting) {
		_move_input *= _sprint_mult;
	}

	glm::vec3 pos_delta = transform.forward() * _move_input.z + transform.right() * _move_input.x + transform.up() * _move_input.y;
	transform.translate(pos_delta * _speed * delta_time);
}

void Camera::_handle_keyboard(const uint8_t *keyboard_state) {
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
	if (keyboard_state[SDL_SCANCODE_SPACE] || keyboard_state[SDL_SCANCODE_E]) {
		_move_input.y += 1;
	}
	if (keyboard_state[SDL_SCANCODE_C] || keyboard_state[SDL_SCANCODE_Q]) {
		_move_input.y -= 1;
	}

	_is_sprinting = keyboard_state[SDL_SCANCODE_LSHIFT];
}
