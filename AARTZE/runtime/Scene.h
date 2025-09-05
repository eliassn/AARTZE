#pragma once
#include "Mesh.h"
#include "GLMesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>

namespace aartze {

struct Transform {
    glm::vec3 t{0};
    glm::vec3 r{0};
    glm::vec3 s{1,1,1};
    glm::mat4 localMatrix() const {
        glm::mat4 M(1);
        M = glm::translate(M, t);
        M = glm::rotate(M, glm::radians(r.x), {1,0,0});
        M = glm::rotate(M, glm::radians(r.y), {0,1,0});
        M = glm::rotate(M, glm::radians(r.z), {0,0,1});
        M = glm::scale(M, s);
        return M;
    }
};

struct MeshInstance {
    std::string name;
    Transform   xform;
    Mesh        cpu;
    GLMesh      gpu;
    int         materialIndex = -1;
};

struct Scene {
    std::vector<Material>     materials;
    std::vector<MeshInstance> meshes;
    int selected = -1;
};

} // namespace aartze

