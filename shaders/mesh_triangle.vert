#include "include/common.glsl"

layout(location = 0) in vec3 vertex_pos;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec3 vertex_color;

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec3 out_pos;
layout(location = 2) out uint out_frame_number;

layout(set = 0, binding = 0) uniform CameraData {
    mat4 view;
    mat4 proj;
    mat4 view_proj;
} camera_data;

layout(push_constant) uniform constants
{
    mat4 model_matrix;
    uint frame_number;
} push_constants;

void main() {
    vec4 pos = vec4(vertex_pos, 1.0f);
    mat4 transform_mat = camera_data.view_proj * push_constants.model_matrix;

    gl_Position = transform_mat * pos;

    out_color = vertex_color;
    out_pos = vertex_pos;
    out_frame_number = push_constants.frame_number;
}
