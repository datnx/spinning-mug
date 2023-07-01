#version 450

vec3 light_pos = vec3(-2.0, 2.0, 2.0);
vec3 light_col = vec3(1.0, 1.0, 1.0);
vec3 eye = vec3(2.0, 2.0, 2.0);

layout(location = 0) in vec3 vertex_pos;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec3 vertex_color;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 cool = vec3(0.0, 0.0, 0.55) + 0.25 * vertex_color;
    vec3 warm = vec3(0.3, 0.3, 0.0) + 0.25 * vertex_color;
    vec3 l = normalize(light_pos - vertex_pos);
    vec3 n = normalize(vertex_normal);
    float t = (dot(n, l) + 1.0) / 2.0;
    vec3 r = 2.0 * dot(n, l) * n - l;
    vec3 v = normalize(eye - vertex_pos);
    float s = clamp(100.0 * dot(r, v) - 97.0, 0.0, 1.0);
    outColor = vec4(mix(mix(cool, warm, t), light_col, s), 1.0);
}