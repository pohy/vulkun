#include "game_objects.h"
#include "material.h"

Monkey::Monkey(Vulkun &vulkun, uint32_t offset) :
		pVulkun(&vulkun), _offset(offset) {
	render_object.pMesh = vulkun.get_mesh(MeshName::Monkey);
	render_object.pMaterial = vulkun.get_material(MaterialName::Lit);
	_apply_next_pos(1.0f);
}

void Monkey::update(float delta_time) {
	// _apply_next_pos(delta_time);
}

void Monkey::_apply_next_pos(float delta_time) {
	uint32_t frame_number = pVulkun->frame_number();
	if (_offset == 0) {
		float sign = glm::sign(sin(frame_number * 0.08f));
		transform.rotate(delta_time * 0.3f, glm::vec3(0, 1, 0));
		transform.rotate(sign * 0.01f, glm::vec3(0, 1, 0));
	} else {
		float rot_delta = sin(_offset + frame_number * delta_time) * 0.1f;
		transform.rotate(rot_delta, transform.right());
	}
}

Triangle::Triangle(Vulkun &vulkun) {
	render_object.pMesh = vulkun.get_mesh(MeshName::Triangle);
	render_object.pMaterial = vulkun.get_material(MaterialName::Lit);
}

Impreza::Impreza(Vulkun &vulkun) {
	pVulkun = &vulkun;
	render_object.pMesh = vulkun.get_mesh(MeshName::Impreza);
	render_object.pMaterial = vulkun.get_material(MaterialName::Impreza);
	render_object.pMaterial->uniforms.albedo_color = glm::vec4{ 0.1f, 0.1f, 0.9f, 1.0f };
}

void Impreza::update(float delta_time) {
	transform.rotate(-delta_time * 0.3f, glm::vec3(0, 1, 0));
	render_object.pMaterial->uniforms.albedo_color.x = abs(sin(pVulkun->frame_number() / 100.0f));
}
