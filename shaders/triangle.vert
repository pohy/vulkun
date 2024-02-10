#version 450

mat4 translate(vec3 offset) {
	// TODO: Why didn't this work?
	// return mat4(
	// 	vec4(1.0f, 0.0f, 0.0f, 0.0f),
	// 	vec4(0.0f, 1.0f, 0.0f, 0.0f),
	// 	vec4(0.0f, 0.0f, 1.0f, 0.0f),
	// 	vec4(offset.x, offset.y, offset.z, 1.0f)
	// );
	return mat4(
		vec4(1.0f, 0.0f, 0.0f, offset.x),
		vec4(0.0f, 1.0f, 0.0f, offset.y),
		vec4(0.0f, 0.0f, 1.0f, offset.z),
		vec4(0.0f, 0.0f, 0.0f, 1.0f)
	);
}

mat4 scale(vec3 scale) {
	return mat4(
		vec4(scale.x, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, scale.y, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, scale.z, 0.0f),
		vec4(0.0f, 0.0f, 0.0f, 1.0f)
	);
}

void main()
{
	const vec3 positions[3] = vec3[3](
		vec3(1.0f, 1.0f, 0.0f),
		vec3(-1.0f, 1.0f, 0.0f),
		vec3(0.0f, -1.0f, 0.0f)
	);

	vec3 pos = positions[gl_VertexIndex];
	//mat4 transformation = translate(vec3(0.0f, -1.0f, 0.0f)) * scale(vec3(0.5f));
	//mat4 transformation = scale(vec3(0.5f));
	mat4 transformation = translate(vec3(0.5f, -0.1f, 0.0f)) * scale(vec3(0.5f));

	gl_Position = vec4(pos, 1.0f) * transformation;
}
