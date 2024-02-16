#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include "mouse.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
private:
	glm::vec3 _move_dir = glm::vec3(0);
	glm::vec2 _rot_amount = glm::vec2(0);

	glm::mat4 _transform = glm::translate(glm::mat4(1.0f), glm::vec3(0, -2, -10));

	float _speed = 10.0f;
	float _sprint_mult = 2.0f;
	bool _is_sprinting = false;
	float _mouse_sens = 0.1f;

	void _handle_keyboard(const uint8_t *keyboard_state);

public:
	float fov = 70.0f;
	float near = 0.1f;
	float far = 200.0f;

	void handle_input(const uint8_t *keyboard_state, Mouse &mouse);

	void update(float delta_time);

	glm::mat4 get_projection(float aspect);

	glm::mat4 get_view() {
		return _transform;
	}

	glm::vec3 get_pos() {
		return glm::vec3(_transform[3]);
	}
};
