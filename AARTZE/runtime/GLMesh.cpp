#include "GLMesh.h"
#include <vector>
using namespace aartze;

void GLMesh::upload(const Mesh& m){
    destroy();
    indexCount = (GLsizei)m.indices.size();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // positions
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, m.positions.size()*sizeof(m.positions[0]), m.positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // normals
    if(!m.normals.empty()){
        glGenBuffers(1, &nbo);
        glBindBuffer(GL_ARRAY_BUFFER, nbo);
        glBufferData(GL_ARRAY_BUFFER, m.normals.size()*sizeof(m.normals[0]), m.normals.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }

    // uvs
    if(!m.uvs0.empty()){
        glGenBuffers(1, &tbo);
        glBindBuffer(GL_ARRAY_BUFFER, tbo);
        glBufferData(GL_ARRAY_BUFFER, m.uvs0.size()*sizeof(m.uvs0[0]), m.uvs0.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }

    // indices
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m.indices.size()*sizeof(uint32_t), m.indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void GLMesh::destroy(){
    if(ibo) glDeleteBuffers(1, &ibo), ibo=0;
    if(tbo) glDeleteBuffers(1, &tbo), tbo=0;
    if(nbo) glDeleteBuffers(1, &nbo), nbo=0;
    if(vbo) glDeleteBuffers(1, &vbo), vbo=0;
    if(vao) glDeleteVertexArrays(1, &vao), vao=0;
    indexCount = 0;
}

void GLMesh::draw() const{
    if(!vao || !indexCount) return;
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);
}

