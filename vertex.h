#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <array>

struct VertexBase {
    /*
    A base vertex. Every vertex will have a position, a normal, and texture coordinates.
    */
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texCoord;

    VertexBase();
    VertexBase(glm::vec3 p, glm::vec3 n, glm::vec2 uv);
};

struct Vertex : public VertexBase {
    /*
    Implementing the base Vertex class
    */

    Vertex();

    Vertex(glm::vec3 p, glm::vec3 n, glm::vec2 uv);

    static VkVertexInputBindingDescription getBindingDescription();

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
};

struct VertexWithTangent : public VertexBase {
    /*
    A vertex with added tangent vector for normal mapping
    */
    glm::vec3 tangent;

    VertexWithTangent(glm::vec3 p, glm::vec3 n, glm::vec2 uv);

    static VkVertexInputBindingDescription getBindingDescription();

    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions();
};