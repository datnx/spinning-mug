#pragma once

#include <glm/vec4.hpp>
#include <string>

struct UnattenuatedPointLight {
    glm::vec4 pos;
    glm::vec4 col;
};

struct DirectionalLight {
    glm::vec4 dir;
    glm::vec4 col;
};

struct PointLight {
    glm::vec4 pos;
    glm::vec4 col;
    alignas(16) float falloff;
};

struct SpotLight {
    glm::vec4 pos;
    glm::vec4 dir;
    glm::vec4 col;
    float cos_p;
    float cos_u;
    alignas(8) float falloff;
};

class light {
public:
    int num_unattenuated_point_light;
    int num_directional_light;
    int num_point_light;
    int num_spot_light;
    UnattenuatedPointLight unattenuated_point_light[2]; //64 bytes
    DirectionalLight directional_light[2]; // 64 bytes
    PointLight point_light[2]; // 96 bytes
    SpotLight spot_light[2]; // 192 bytes

    light();

    void load_file(std::string file_path);
};