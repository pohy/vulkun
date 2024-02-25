#pragma once

#include "mouse.h"
#include "transform.h"

#include <glm/glm.hpp>

class Camera {
private:
	glm::vec3 _move_input{ 0 };
	glm::vec2 _rot_amount{ 0 };
	glm::vec2 _vertical_clamp{ -89.0f, 89.0f };

	float _speed = 10.0f;
	float _sprint_mult = 2.0f;
	bool _is_sprinting = false;
	float _mouse_sens = 0.3f;

	void _handle_keyboard(const uint8_t *keyboard_state);

public:
	Transform transform;

	float fov = 70.0f;
	float near = 0.1f;
	float far = 200.0f;

	void handle_input(const uint8_t *keyboard_state, Mouse &mouse);

	void update(float delta_time);

	glm::mat4 get_projection(float aspect);

	glm::mat4 get_view();

	glm::vec3 get_pos() { return transform.pos(); }

	Camera() {
		transform.set_pos(glm::vec3{ 0, -2, 10 });
	}
};
