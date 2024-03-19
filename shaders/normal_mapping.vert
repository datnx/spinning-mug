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
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec3 vertex_pos;
layout(location = 1) out vec3 vertex_normal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 vertex_tangent;

void main() {
    
    // calculate vertex position in the clip space
    gl_Position = vp.proj * vp.view * model_matrix.model * vec4(inPosition, 1.0);
    vertex_pos = (model_matrix.model * vec4(inPosition, 1.0)).xyz;
    vertex_normal = mat3(model_matrix.model) * inNormal;
    fragTexCoord = inTexCoord;
    vertex_tangent = (model_matrix.model * vec4(inTangent, 1.0)).xyz;
}