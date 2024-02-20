#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include "mouse.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
private:
	glm::vec3 _move_input{ 0 };
	glm::vec2 _rot_amount{ 0 };

	glm::vec3 _pos{ 0, -2, 10 };

	glm::vec3 _forward{ 0, 0, -1 };
	glm::vec3 _up{ 0, 1, 0 };
	glm::vec3 _right{ 1, 0, 0 };

	float _yaw = -90.0f;
	float _pitch = 0.0f;

	float _speed = 10.0f;
	float _sprint_mult = 2.0f;
	bool _is_sprinting = false;
	float _mouse_sens = 5.0f;

	void _handle_keyboard(const uint8_t *keyboard_state);

public:
	float fov = 70.0f;
	float near = 0.1f;
	float far = 200.0f;

	void handle_input(const uint8_t *keyboard_state, Mouse &mouse);

	void update(float delta_time);

	glm::mat4 get_projection(float aspect);

	glm::mat4 get_view();

	glm::vec3 get_pos() { return _pos; }
};
