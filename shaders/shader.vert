#version 450

layout(set = 0, binding = 0) uniform ViewProjection {
    mat4 view;
    mat4 proj;
} vp;

layout(set = 2, binding = 0) uniform Model {
    mat4 model;
} model_matrix;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 vertex_pos;
layout(location = 1) out vec3 vertex_normal;
layout(location = 2) out vec4 vertex_color;
layout(location = 3) out vec2 fragTexCoord;

void main() {
    gl_Position = vp.proj * vp.view * model_matrix.model * vec4(inPosition, 1.0);
    vertex_pos = (model_matrix.model * vec4(inPosition, 1.0)).xyz;
    vertex_normal = mat3(model_matrix.model) * inNormal;
    vertex_color = inColor;
    fragTexCoord = inTexCoord;
}