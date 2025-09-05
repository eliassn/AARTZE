#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include <glad/glad.h>

namespace aartze {

class Renderer {
public:
    void initGL();
    void setViewport(int x, int y, int w, int h);

    void setPerspective(float fovY_deg, float aspect, float znear, float zfar);
    void setViewLookAt(const glm::vec3& eye, const glm::vec3& at, const glm::vec3& up);

    void getViewMatrix(float out16[16]) const;
    void getProjMatrix(float out16[16]) const;

    void drawGrid(float extent, float step);
    void drawCube(const float model[16], uint32_t rgba);

    // Accessors for UI integration
    const glm::mat4& view() const { return m_view; }
    const glm::mat4& proj() const { return m_proj; }

    // M3 additions: simple lit pipeline used by GLMesh draws
    void useLitShader();
    void setVP(const glm::mat4& V, const glm::mat4& P);
    void setModelColor(const glm::mat4& M, const glm::vec4& color);
    void draw(GLuint vao, GLsizei indexCount);

    // Viewport texture glue (stubbed until a real FBO is added)
    GLuint GetViewportTexture() const { return 0; }
    void   ResizeViewport(int /*w*/, int /*h*/) {}

private:
    unsigned m_progColor = 0;
    unsigned m_progLit   = 0;
    unsigned m_vboGrid   = 0;
    unsigned m_vaoGrid   = 0;
    unsigned m_vboCube   = 0;
    unsigned m_vaoCube   = 0;
    int m_uMVP = -1;
    // lit uniforms
    int m_uM = -1, m_uV = -1, m_uP = -1, m_uN = -1, m_uLitColor=-1;
    glm::mat4 m_view{1.0f};
    glm::mat4 m_proj{1.0f};
};

}
