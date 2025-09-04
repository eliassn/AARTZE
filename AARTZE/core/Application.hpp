#pragma once
#define GLFW_INCLUDE_NONE  // Prevent GLFW from including OpenGL headers
#include <glad/glad.h>

#include "../input/InputHandler.hpp"
#include "../systems/RenderingSystem/RenderingSystem.hpp"
#include "GLFW/glfw3.h"
#include "SystemManager.hpp"
#include "ECSPreset.hpp"
// Legacy ImGui UI removed in favor of UE-like shell
#include "../audio/AudioSystem.hpp"
// AARTZE UI2 (Slate-like) shell (optional)
#ifdef USE_UI2
#  include "../ui2/EditorShell.hpp"
#else
namespace ui2 {
struct EditorShell {
    void Initialize(GLFWwindow*) {}
    void Resize(int, int) {}
    void Tick(float) {}
    void Render() {}
    void Shutdown() {}
};
}
#endif

#ifdef AARTZE_WITH_IMGUI
#  include <imgui.h>
#  include <backends/imgui_impl_glfw.h>
#  include <backends/imgui_impl_opengl3.h>
#  include "../editor/AartzeEditor.h"
#endif

class Application
{
   public:
    Application(int width = 800, int height = 600, const char* title = "AARTZE Game");
    ~Application();
    void Run();

   private:
    void InitWindow(int width, int height, const char* title);

    GLFWwindow* m_window{nullptr};
    RenderingSystem m_renderingSystem;
    InputHandler m_inputHandler;
    // UIManager removed
    AudioSystem m_audio;
    ui2::EditorShell m_editor;

#ifdef AARTZE_WITH_IMGUI
    bool m_imguiReady{false};
    aartze::EditorUI m_aartzeUI;
#endif
};
