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
		_forward = glm::normalize(glm::vec3(
				cos(glm::radians(_rot.y)) * cos(glm::radians(_rot.x)),
				sin(glm::radians(_rot.x)),
				sin(glm::radians(_rot.y)) * cos(glm::radians(_rot.x))));
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
};
