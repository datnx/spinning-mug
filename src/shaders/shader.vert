#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;

layout(location = 0) out vec3 vertex_pos;
layout(location = 1) out vec3 vertex_normal;
layout(location = 2) out vec3 vertex_color;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    vertex_pos = (ubo.model * vec4(inPosition, 1.0)).xyz;
    vertex_normal = (ubo.model * vec4(inNormal, 1.0)).xyz;
    vertex_color = inColor;
}