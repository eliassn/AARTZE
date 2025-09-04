#pragma once
#include <glad/glad.h>

struct Fullscreen
{
    GLuint vao{0}, vbo{0};
    void Create(){ if(vao) return; float v[]={-1,-1, 1,-1, 1,1, -1,-1, 1,1, -1,1}; glGenVertexArrays(1,&vao); glGenBuffers(1,&vbo); glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER,vbo); glBufferData(GL_ARRAY_BUFFER,sizeof(v),v,GL_STATIC_DRAW); glEnableVertexAttribArray(0); glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,2*sizeof(float),(void*)0); glBindVertexArray(0);} 
    void Destroy(){ if(vbo) glDeleteBuffers(1,&vbo); if(vao) glDeleteVertexArrays(1,&vao); vao=vbo=0; }
    void Draw(){ glBindVertexArray(vao); glDrawArrays(GL_TRIANGLES,0,6);} 
};

