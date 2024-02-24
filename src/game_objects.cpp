#include "game_objects.h"

Monkey::Monkey(Vulkun &vulkun) :
		pVulkun(&vulkun) {
	render_object.pMesh = vulkun.get_mesh(MeshName::Monkey);
	render_object.pMaterial = vulkun.get_material(MaterialName::Default);
}

void Monkey::update(float delta_time) {
	glm::vec3 rot = transform.rot();
	rot.x = sin(pVulkun->frame_number() * 0.01f) * 40.0f;
	transform.set_rot(rot);
}

Triangle::Triangle(Vulkun &vulkun) {
	render_object.pMesh = vulkun.get_mesh(MeshName::Triangle);
	render_object.pMaterial = vulkun.get_material(MaterialName::Default);
}
