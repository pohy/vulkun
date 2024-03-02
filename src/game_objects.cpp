#include "game_objects.h"

Monkey::Monkey(Vulkun &vulkun, uint32_t offset) :
		pVulkun(&vulkun), _offset(offset) {
	render_object.pMesh = vulkun.get_mesh(MeshName::Monkey);
	render_object.pMaterial = vulkun.get_material(MaterialName::Default);
}

void Monkey::update(float delta_time) {
	uint32_t frame_number = pVulkun->frame_number();
	if (_offset == 0) {
		float sign = glm::sign(sin(frame_number * 0.08f));
		transform.rotate(delta_time * 0.3f, glm::vec3(0, 1, 0));
		transform.rotate(sign * 0.01f, glm::vec3(0, 1, 0));
	} else {
		glm::vec3 eulers = transform.rot_euler();
		float rot_range = glm::radians(30.0f);
		float rot_delta = sin(_offset + frame_number * delta_time) * 0.1f;
		transform.rotate(rot_delta, transform.right());
	}
}

Triangle::Triangle(Vulkun &vulkun) {
	render_object.pMesh = vulkun.get_mesh(MeshName::Triangle);
	render_object.pMaterial = vulkun.get_material(MaterialName::ShiftingColors);
}

Impreza::Impreza(Vulkun &vulkun) {
	render_object.pMesh = vulkun.get_mesh(MeshName::Impreza);
	render_object.pMaterial = vulkun.get_material(MaterialName::Default);
}
