#version 450

layout (location = 0) in vec3 inColor;

layout (location = 0) out vec4 outFragColor;

void main()
{
    //outFragColor = vec4(0.7f, 0.3f, 0.1f, 1.0f);
	outFragColor = vec4(inColor, 1.0f);
}
