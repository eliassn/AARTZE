#pragma once
#include <glad/glad.h>

struct GBuffer
{
    GLuint fbo{0};
    GLuint texAlbedoMR{0}; // rgba: albedo.rgb, (a not used); MR in separate?
    GLuint texNormal{0};   // xyz: view-space normal, w: unused
    GLuint texDepth{0};    // depth texture (GL_DEPTH_COMPONENT)
    int width{0}, height{0};

    void Create(int w, int h);
    void Destroy();
    void Resize(int w, int h);
};

