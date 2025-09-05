#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <glm/glm.hpp>

namespace aartze {

struct Submesh {
    uint32_t firstIndex = 0;
    uint32_t indexCount = 0;
    int      materialIndex = -1;
};

struct Mesh {
    std::string name;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec4> tangents;   // xyz + handedness
    std::vector<glm::vec2> uvs0;
    std::vector<uint32_t>  indices;
    std::vector<Submesh>   submeshes;
};

struct Material {
    std::string name;
    glm::vec3   baseColor{1,1,1};
    float       roughness = 0.7f;
    float       metallic  = 0.0f;
};

} // namespace aartze

