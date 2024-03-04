#include "include/common.glsl"

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec3 in_pos;
layout(location = 2) flat in uint in_frame_number;

layout(location = 0) out vec4 out_frag_color;

void main()
{
    vec3 subaru_blue = vec3(0.012, 0.208, 0.58);

    out_frag_color = vec4(subaru_blue + in_color * 0.2f, 1.0f);
}
