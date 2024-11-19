#version 450

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_frag_pos;

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
    vec3 object_color = in_color;
    vec3 normal = normalize(in_normal);
    vec3 light_dir = normalize(scene_data.sun_direction.xyz - in_frag_pos);
    float diffuse = max(dot(normal, light_dir), 0.0);
    vec3 diffuse_color = scene_data.sun_color.xyz * diffuse;

    vec3 ambient_color = scene_data.ambient_color.xyz * scene_data.ambient_color.w;
    vec3 final_color = (ambient_color + diffuse_color) * object_color;

    out_frag_color = vec4(final_color, 1.0f);
}
