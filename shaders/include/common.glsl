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

float pos_sin(float x) {
	return sin(x) * 0.5f + 0.5f;
}
