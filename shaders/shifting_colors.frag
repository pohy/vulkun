#include "include/common.glsl"

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec3 in_pos;
layout(location = 2) flat in uint in_frame_number;

layout(location = 0) out vec4 out_frag_color;

void main()
{
    //out_frag_color = vec4(0.7f, 0.3f, 0.1f, 1.0f);
    vec3 offset = vec3(in_pos) + vec3(in_frame_number);

    out_frag_color = vec4(in_color, 1.0f);
    out_frag_color.x *= pos_sin(offset.x / 100.0f);
    out_frag_color.y *= pos_sin(offset.z / 10.0f);
    out_frag_color.z *= pos_sin(offset.y / 1000.0f);
}
