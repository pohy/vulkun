#version 450

layout(location = 0) in vec3 in_color;

layout(location = 0) out vec4 out_frag_color;

layout(set = 0, binding = 1) uniform SceneData {
    vec4 sun_direction;
    vec4 sun_color;
    vec4 ambient_color; // w for intensity
    vec4 fog_color; // w for exponent
    vec4 fog_distances; // x for start, y for end, zw unused
} scene_data;

void main()
{
    out_frag_color = vec4(in_color + scene_data.ambient_color.xyz * scene_data.ambient_color.w, 1.0f);
    // out_frag_color = vec4(scene_data.ambient_color.xyz, 1.0f);
}
