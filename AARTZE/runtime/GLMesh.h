#pragma once
#include "Mesh.h"
#include <glad/glad.h>

namespace aartze {

struct GLMesh {
    GLuint vao = 0, vbo = 0, nbo = 0, tbo = 0, ibo = 0;
    GLsizei indexCount = 0;
    bool valid() const { return vao != 0; }
    void upload(const Mesh& m);
    void destroy();
    void draw() const; // draws whole mesh
};

} // namespace aartze

