#include "include/common.glsl"

layout (location = 0) in vec3 vertex_pos;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec3 vertex_color;

layout (location = 0) out vec3 out_color;

layout (push_constant) uniform constants
{
	mat4 render_matrix;
	uint frame_number;
} PushConstants;

void main() {
	float current_scale = 0.5f + pos_sin(PushConstants.frame_number * 0.01f);
	mat4 transformation = scale(vec3(current_scale));// * translate(vec3(-0.5f, 0.0f, 0.0f));
	vec4 pos = vec4(vertex_pos, 1.0f) * transformation;
	gl_Position = PushConstants.render_matrix * pos;
	out_color = vertex_color;
}

