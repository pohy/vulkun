#pragma once

#include "material.h"
#include "mesh.h"
#include "transform.h"
#include "vulkun.h"

#include <vulkan/vulkan.h>

class Vulkun;

struct RenderObject {
	Mesh *pMesh = nullptr;
	Material *pMaterial = nullptr;
};

struct IGameObject {
	// TODO: Queue free
	Transform transform;
	RenderObject render_object;

	IGameObject() = default;
	virtual ~IGameObject() = default;

	virtual void update(float delta_time){};

	virtual std::string name() = 0;
};

class Monkey : public IGameObject {
private:
	Vulkun *pVulkun;
	uint32_t _offset;

public:
	// TODO: Coupling to Vulkun doesn't seem right
	Monkey(Vulkun &vulkun, uint32_t offset);

	virtual void update(float delta_time) override;

	virtual std::string name() override { return "Monkey"; }
};

class Triangle : public IGameObject {
public:
	Triangle(Vulkun &vulkun);

	virtual std::string name() override { return "Triangle"; }
};

class Impreza : public IGameObject {
public:
	Impreza(Vulkun &vulkun);

	virtual std::string name() override { return "Impreza"; }

	virtual void update(float delta_time) override;
};
