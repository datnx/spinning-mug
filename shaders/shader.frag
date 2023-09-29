#version 450

struct UnattenuatedPointLight {
    vec4 pos;
    vec4 col;
};

struct DirectionalLight {
    vec4 dir;
    vec4 col;
};

struct PointLight {
    vec4 pos;
    vec4 col;
    float falloff;
};

struct SpotLight {
    vec4 pos;
    vec4 dir;
    vec4 col;
    float cos_p;
    float cos_u;
    float falloff;
};

layout(set = 0, binding = 1) uniform UniformBufferObject {
    int num_pna;
    int num_dir;
    int num_pwa;
    int num_spo;
    UnattenuatedPointLight pna[2];
    DirectionalLight dir[2];
    PointLight pwa[2];
    SpotLight spo[2];
    vec3 eye;
} ubo;

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec3 vertex_pos;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec4 vertex_color;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

vec3 lit(vec3 l, vec3 n, vec3 v, vec3 warm) {
    vec3 r_l = reflect(-l, n);
    float s = clamp(100.0 * dot(r_l, v) - 97.0, 0.0, 1.0);
    vec3 highlightColor = vec3(1.0, 1.0, 1.0);
    return mix(warm, highlightColor, s);
}

void main() {
    vec4 texture_color = texture(texSampler, fragTexCoord);
    vec3 cool = vec3(0.0, 0.0, 0.1) + 0.5 * texture_color.rgb;
    vec3 warm = vec3(0.1, 0.1, 0.0) + 0.5 * texture_color.rgb;
    vec3 v = normalize(ubo.eye - vertex_pos);
    vec3 n = normalize(vertex_normal);
    outColor = vec4(0.5 * cool, texture_color.a);
    for (int i = 0; i < ubo.num_pna; i++) {
        vec3 l = normalize(ubo.pna[i].pos.xyz - vertex_pos);
        float Ndl = clamp(dot(n, l), 0.0, 1.0);
        outColor.rgb += Ndl * ubo.pna[i].col.rgb * lit(l, n, v, warm);
    }
    for (int i = 0; i < ubo.num_dir; i++) {
        vec3 l = normalize(-ubo.dir[i].dir.xyz);
        float Ndl = clamp(dot(n, l), 0.0, 1.0);
        outColor.rgb += Ndl * ubo.dir[i].col.rgb * lit(l, n, v, warm);
    }
    for (int i = 0; i < ubo.num_pwa; i++) {
        vec3 l = ubo.pwa[i].pos.xyz - vertex_pos;
        float r_2 = dot(l, l);
        float r = sqrt(r_2);
        l = l / r;
        float Ndl = clamp(dot(n, l), 0.0, 1.0);
        float attenuation_factor = pow(clamp(1 - pow(r / ubo.pwa[i].falloff, 4), 0.0, 1.0), 2) / (1 + r_2);
        outColor.rgb += Ndl * attenuation_factor * ubo.pwa[i].col.rgb * lit(l, n, v, warm);
    }
    for (int i = 0; i < ubo.num_spo; i++) {
        vec3 l = ubo.spo[i].pos.xyz - vertex_pos;
        float r_2 = dot(l, l);
        float r = sqrt(r_2);
        l = l / r;
        float Ndl = clamp(dot(n, l), 0.0, 1.0);
        float attenuation_factor = pow(clamp(1 - pow(r / ubo.spo[i].falloff, 4), 0.0, 1.0), 2) / (1 + r_2);
        float t = clamp((dot(normalize(ubo.spo[i].dir.xyz), -l) - ubo.spo[i].cos_u) / (ubo.spo[i].cos_p - ubo.spo[i].cos_u), 0.0, 1.0);
        float directional_falloff_factor = t * t * (3.0 - 2.0 * t);
        outColor.rgb += Ndl * attenuation_factor * directional_falloff_factor * ubo.spo[i].col.rgb * lit(l, n, v, warm);
    }
}