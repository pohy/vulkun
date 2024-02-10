#version 450

#include "include/common.glsl"

layout(location = 0) out vec3 outColor;

void main()
{
	const vec3 positions[3] = vec3[3](
		vec3(1.0f, 1.0f, 0.0f),
		vec3(-1.0f, 1.0f, 0.0f),
		vec3(0.0f, -1.0f, 0.0f)
	);

	const vec3 colors[3] = vec3[3](
		vec3(1.0f, 0.0f, 0.0f),
		vec3(0.0f, 1.0f, 0.0f),
		vec3(0.0f, 0.0f, 1.0f)
	);

	vec3 pos = positions[gl_VertexIndex];
	mat4 transformation = translate(vec3(0.0f, -0.3f, 0.0f)) * scale(vec3(0.5f));

	gl_Position = vec4(pos, 1.0f) * transformation;
	outColor = colors[gl_VertexIndex];
}
