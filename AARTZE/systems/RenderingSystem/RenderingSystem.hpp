#pragma once
#include <cstdint>
struct GLFWwindow;

class RenderingSystem
{
public:
    bool Initialize(GLFWwindow* window);
    void Shutdown();
    void Render();
    void GetView(float out[16]) const;  // column-major
    void GetProj(float out[16], int w, int h) const;
    const float* GetCameraPos() const { return m_camPos; }

private:
    GLFWwindow* m_window{nullptr};
    unsigned m_program{0};
    float m_camPos[3] {0.0f, 1.5f, 3.0f};
    float m_camYaw{ -90.0f };
    float m_camPitch{ -10.0f };
    bool  m_mouseLook{ false };
    float m_camSpeed{ 2.0f };
    bool  m_useDeferred{ true };
    bool  m_ssao{ true };
    bool  m_shadows{ false };
    bool  m_ssr{ false };

    void ensureProgram();

    // Deferred renderer
    class DeferredRenderer* m_deferred{nullptr};
public:
    void SetCameraSpeed(float s) { m_camSpeed = s; }
private:
    unsigned m_lineProg{0};
    unsigned m_gridVao{0}, m_gridVbo{0};
    void ensureLineProgram();
    void drawGridAndAxes();
    void drawGizmo();
};
