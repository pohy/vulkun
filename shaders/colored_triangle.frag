#version 450

layout (location = 0) in vec3 in_color;

layout (location = 0) out vec4 out_frag_color;

void main()
{
    //out_frag_color = vec4(0.7f, 0.3f, 0.1f, 1.0f);
    out_frag_color = vec4(in_color * 1.2f, 1.0f);
}
