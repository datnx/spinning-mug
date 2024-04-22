#version 450

layout(set = 0, binding = 0) uniform vec3 translate;

layout(location = 0) in vec3 inPosition;

void main() {
    gl_Position = vec4(inPosition + translate, 1.0);
}