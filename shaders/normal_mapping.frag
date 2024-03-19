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

layout(set = 2, binding = 1) uniform sampler2D norSampler;

layout(location = 0) in vec3 vertex_pos;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 vertex_tangent;

layout(location = 0) out vec4 outColor;

vec3 lit(vec3 l, vec3 n, vec3 v, vec3 warm) {
    vec3 r_l = reflect(-l, n);
    float s = clamp(100.0 * dot(r_l, v) - 97.0, 0.0, 1.0);
    vec3 highlightColor = vec3(1.0, 1.0, 1.0);
    return mix(warm, highlightColor, s);
}

void main() {

    // 1. Calculate TBN matrix (TBN stands for tangent, bitangent, normal)

    // 1.1. Normalize the normal vector and the tangent vector
    vec3 new_normal = normalize(vertex_normal);
    vec3 norm_tangent = normalize(vertex_tangent);

    // 1.2. Calculate a new tangent vector that is perpendicular to the normalized normal vector
    // and align with the tangent vector
    vec3 new_tangent = normalize(norm_tangent - dot(new_normal, norm_tangent) * new_normal);

    // 1.3. The bitangent vector is obtained by taking the cross product of the new normal and the new tangent
    vec3 new_bitangent = cross(new_normal, new_tangent);

    // 1.4. Assemble the TBN matrix
    mat3 TBN = mat3(new_tangent, new_bitangent, new_normal);

    // 2. Get the normal vector in tangent space from the normal map
    vec3 normal_tangent_space = texture(norSampler, fragTexCoord).rgb;

    // 3. Convert the normal vector in 2. from [0:1] to [-1:1]
    normal_tangent_space = normal_tangent_space * 2 - 1;

    // 4. Transform the normal vector from tangent space to the world space using the TBN matrix
    vec3 normal_world = TBN * normal_tangent_space;

    vec4 texture_color = texture(texSampler, fragTexCoord);
    vec3 cool = vec3(0.0, 0.0, 0.1) + 0.5 * texture_color.rgb;
    vec3 warm = vec3(0.1, 0.1, 0.0) + 0.5 * texture_color.rgb;
    vec3 v = normalize(ubo.eye - vertex_pos);
    vec3 n = normalize(normal_world);
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