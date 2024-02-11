#include "include/common.glsl"

layout (location = 0) in vec3 vertex_pos;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec3 vertex_color;

layout (location = 0) out vec3 outColor;

void main() {
	mat4 transformation = scale(vec3(0.75f)) * translate(vec3(-0.5f, 0.0f, 0.0f));
	vec4 pos = vec4(vertex_pos, 1.0f) * transformation;
	gl_Position = pos;
	outColor = vertex_color;
}

