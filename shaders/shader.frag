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
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

vec3 cal_unattenuated_point_light(UnattenuatedPointLight light, vec3 diffuse_color, vec3 n, vec3 v) {
    
    vec3 l = normalize(light.pos.xyz - vertex_pos);
    vec3 h = normalize(v + l);
    
    vec3 diffuse = light.col.rgb * diffuse_color * max(dot(n, l), 0);

    vec3 specular = light.col.rgb * vec3(0.04) * pow(max(dot(n, h), 0), 10);

    return diffuse + specular;
}

vec3 cal_directional_light(DirectionalLight light, vec3 diffuse_color, vec3 n, vec3 v) {
    
    vec3 l = normalize(-light.dir.xyz);
    vec3 h = normalize(v + l);
    
    vec3 diffuse = light.col.rgb * diffuse_color * max(dot(n, l), 0);

    vec3 specular = light.col.rgb * vec3(0.04) * pow(max(dot(n, h), 0), 10);

    return diffuse + specular;
}

vec3 cal_point_light(PointLight light, vec3 diffuse_color, vec3 n, vec3 v) {
    
    vec3 l = light.pos.xyz - vertex_pos;
    float r_2 = dot(l, l);
    float r = sqrt(r_2);
    l = l / r;
    vec3 h = normalize(v + l);

    float attenuation = pow(max(1 - pow(r / light.falloff, 4), 0), 2) / (1 + r_2);

    vec3 diffuse = light.col.rgb * diffuse_color * attenuation * max(dot(n, l), 0);

    vec3 specular = light.col.rgb * vec3(0.04) * attenuation * pow(max(dot(n, h), 0), 10);

    return diffuse + specular;
}

vec3 cal_spot_light(SpotLight light, vec3 diffuse_color, vec3 n, vec3 v) {
    
    vec3 l = light.pos.xyz - vertex_pos;
    float r_2 = dot(l, l);
    float r = sqrt(r_2);
    l = l / r;
    vec3 h = normalize(v + l);

    float attenuation = pow(max(1 - pow(r / light.falloff, 4), 0), 2) / (1 + r_2);
    float t = clamp((dot(normalize(light.dir.xyz), -l) - light.cos_u) / (light.cos_p - light.cos_u), 0, 1);
    float directional_falloff = t * t * (3.0 - 2.0 * t);

    vec3 diffuse = light.col.rgb * diffuse_color * attenuation * directional_falloff * max(dot(n, l), 0);

    vec3 specular = light.col.rgb * vec3(0.04) * attenuation * directional_falloff * pow(max(dot(n, h), 0), 10);

    return diffuse + specular;
}

void main() {
    
    // read the texture
    vec4 texture_color = texture(texSampler, fragTexCoord);

    vec3 n = normalize(vertex_normal);
    vec3 v = normalize(ubo.eye - vertex_pos);
    
    // constant ambient
    outColor = texture_color;
    outColor.rgb *= 0.01;

    // unattenuated point light
    for (int i = 0; i < ubo.num_pna; i++) {
        outColor.rgb += cal_unattenuated_point_light(ubo.pna[i], texture_color.rgb, n, v);
    }

    // directional light
    for (int i = 0; i < ubo.num_dir; i++) {
        outColor.rgb += cal_directional_light(ubo.dir[i], texture_color.rgb, n, v);
    }

    // point light
    for (int i = 0; i < ubo.num_pwa; i++) {
        outColor.rgb += cal_point_light(ubo.pwa[i], texture_color.rgb, n, v);
    }

    // spot light
    for (int i = 0; i < ubo.num_spo; i++) {
        outColor.rgb += cal_spot_light(ubo.spo[i], texture_color.rgb, n, v);
    }
}