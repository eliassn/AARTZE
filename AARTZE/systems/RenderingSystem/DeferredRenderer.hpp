#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "GBuffer.hpp"
#include "Fullscreen.hpp"

class DeferredRenderer
{
public:
    bool Initialize();
    void Shutdown();
    void Resize(int w, int h);
    void GeometryPass(const float* Proj, const float* View);
    void LightingPass(const float* Proj, const float* View, const float* CamPos);

    // toggles
    bool enableSSAO{true};
    bool enableShadows{false};
    bool enableSSR{false};

private:
    GBuffer m_gbuf;
    Fullscreen m_fs;
    GLuint m_geomProg{0};
    GLuint m_lightProg{0};

    void ensurePrograms();
};

