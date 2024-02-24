#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transform {
private:
	glm::vec3 _pos{ 0 };
	glm::vec3 _rot{ 0 };
	glm::vec3 _scale{ 1 };

	glm::vec3 _forward{ 0, 0, -1 };
	glm::vec3 _up{ 0, 1, 0 };
	glm::vec3 _right{ 1, 0, 0 };

	void _update_directions() {
		glm::vec3 rot_rad = glm::radians(_rot);

		_forward = glm::normalize(glm::vec3(
				cos(rot_rad.y) * cos(rot_rad.x),
				sin(rot_rad.x),
				sin(rot_rad.y) * cos(rot_rad.x)));

		_right = glm::normalize(glm::cross(_forward, _up));

		_up = glm::normalize(glm::cross(_right, _forward));
	}

public:
	const glm::vec3 pos() const { return _pos; }
	const glm::vec3 rot() const { return _rot; }
	const glm::vec3 scale() const { return _scale; }

	const glm::vec3 forward() const { return _forward; }
	const glm::vec3 up() const { return _up; }
	const glm::vec3 right() const { return _right; }

	void set_pos(glm::vec3 pos) { _pos = pos; }
	void translate(glm::vec3 pos) { _pos += pos; }

	/**
	 * @param rot The new rotation, in degrees.
	 */
	void set_rot(glm::vec3 rot) {
		_rot = rot;
		_update_directions();
	}

	/**
	 * @param angle The angle to rotate by, in degrees.
	 * @param axis The axis to rotate around.
	 */
	void rotate(float angle, glm::vec3 axis) {
		_rot += angle * glm::normalize(axis);
		_rot.x = fmod(_rot.x, 360.0f);
		_rot.y = fmod(_rot.y, 360.0f);
		_rot.z = fmod(_rot.z, 360.0f);

		_update_directions();
	}

	void set_forward(glm::vec3 forward) { _forward = forward; }
	void set_right(glm::vec3 right) { _right = right; }
	void set_up(glm::vec3 up) { _up = up; }

	void set_scale(float scale) { _scale = glm::vec3(scale); }
	void set_scale(glm::vec3 scale) { _scale = scale; }

	glm::mat4 get_model() const {
		glm::mat4 model = glm::mat4(1.0f);

		model = glm::translate(model, _pos);

		model = glm::rotate(model, glm::radians(_rot.x), glm::vec3(1, 0, 0));
		model = glm::rotate(model, glm::radians(_rot.y), glm::vec3(0, 1, 0));
		model = glm::rotate(model, glm::radians(_rot.z), glm::vec3(0, 0, 1));

		model = glm::scale(model, _scale);

		return model;
	}
};
