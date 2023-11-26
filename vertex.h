#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <array>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texCoord;

    Vertex(glm::vec3 p, glm::vec3 n, glm::vec2 uv);

    static VkVertexInputBindingDescription getBindingDescription();

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
};

struct VertexWithTangent {
    /*
    A vertex with added tangent vector for normal mapping
    */

    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec2 texCoord;

    VertexWithTangent(glm::vec3 p, glm::vec3 n, glm::vec3 t, glm::vec2 uv);

    static VkVertexInputBindingDescription getBindingDescription();

    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions();
};