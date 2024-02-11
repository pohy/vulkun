layout (location = 0) in vec3 vertex_pos;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec3 vertex_color;

layout (location = 0) out vec3 outColor;

void main() {
	const vec3 positions[3] = vec3[3](
		vec3(1.0f, 1.0f, 0.0f),
		vec3(-1.0f, 1.0f, 0.0f),
		vec3(0.0f, -1.0f, 0.0f)
	);

	//gl_Position = vec4(vertex_pos, 1.0f);
	gl_Position = vec4(positions[gl_VertexIndex], 1.0f);
	outColor = vertex_color;
	//outColor = vec3(1.0f, 0.0f, 0.0f);
}

