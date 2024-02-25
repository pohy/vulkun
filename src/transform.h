#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class Transform {
private:
	glm::vec3 _pos{ 0 };
	glm::quat _rot = glm::identity<glm::quat>();
	glm::vec3 _scale{ 1 };

	glm::vec3 _right{ 0 }, _up{ 0 }, _forward{ 0 };

	void _update_vectors() {
		_right = glm::normalize(glm::vec3{ _rot * glm::vec3{ 1, 0, 0 } });
		_up = glm::normalize(glm::vec3{ _rot * glm::vec3{ 0, 1, 0 } });
		_forward = glm::normalize(glm::vec3{ _rot * glm::vec3{ 0, 0, -1 } });
	}

public:
	Transform() { _update_vectors(); }

	glm::vec3 pos() const { return _pos; }
	glm::vec3 scale() const { return _scale; }

	glm::quat rot() const { return _rot; }
	glm::vec3 rot_euler() const { return glm::eulerAngles(_rot); }

	glm::vec3 forward() const { return _forward; }
	glm::vec3 up() const { return _up; }
	glm::vec3 right() const { return _right; }

	void set_pos(const glm::vec3 pos) { _pos = pos; }
	void translate(const glm::vec3 translation) { _pos += translation; }

	void set_scale(const float scale) { _scale = glm::vec3{ scale }; }
	void set_scale(const glm::vec3 scale) { _scale = scale; }

	void set_rot(const glm::quat rot) {
		_rot = glm::normalize(rot);
		_update_vectors();
	}

	void rotate(const float angle_rad, const glm::vec3 axis) {
		glm::quat new_rot = glm::rotate(glm::identity<glm::quat>(), angle_rad, axis);
		// The order of multiplication matters!
		// See Ogre's `Node::rotate` implementation for reference: https://github.com/OGRECave/ogre/blob/master/OgreMain/src/OgreNode.cpp#L413
		_rot = glm::normalize(new_rot * _rot);
		_update_vectors();
	}

	void look_at(const glm::vec3 target) {
		_rot = glm::quatLookAt(glm::normalize(_pos - target), _up);
		_update_vectors();
	}

	glm::mat4 get_model() const {
		return glm::translate(glm::mat4{ 1 }, _pos) * glm::mat4{ _rot } * glm::scale(glm::mat4{ 1 }, _scale);
	}
};
