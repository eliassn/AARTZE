#include "Application.hpp"
#define GLFW_INCLUDE_NONE  // Prevent GLFW from including OpenGL headers
#include <glad/glad.h>

#include <stdexcept>

#include "../input/InputHandler.hpp"
#include "GLFW/glfw3.h"
#include "Profiler.hpp"

#include "../scripting/LuaVM.hpp"

static void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void Application::InitWindow(int width, int height, const char* title)
{
    if (!glfwInit()) throw std::runtime_error("Failed to initialize GLFW");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!m_window)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(m_window);
    glfwSetFramebufferSizeCallback(m_window, FramebufferSizeCallback);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    int fbw, fbh;
  glfwGetFramebufferSize(m_window, &fbw, &fbh);
  glViewport(0, 0, fbw, fbh);

#ifdef AARTZE_WITH_IMGUI
  // Initialize Dear ImGui context and backends
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  ImGui_ImplGlfw_InitForOpenGL(m_window, true);
  ImGui_ImplOpenGL3_Init("#version 330");
  aartze::EditorUI::ApplyTheme();
  m_imguiReady = true;
#endif
}

Application::Application(int width, int height, const char* title)
{
    InitWindow(width, height, title);
    m_inputHandler.SetWindow(m_window);
    
    // Legacy ImGui UI removed; initialize UE-like shell
    m_editor.Initialize(m_window);
    // Register minimal components and initialize renderer
    RegisterBasicComponents();
    m_renderingSystem.Initialize(m_window);
    m_audio.Initialize();
    gSystemManager = std::make_unique<SystemManager>();
    gSystemManager->textRenderingSystem.Initialize();
    gSystemManager->physicsSystem.Initialize();
    gSystemManager->navigationSystem.Initialize();
    gLua = std::make_unique<LuaVM>();
    gLua->Init();
}

Application::~Application()
{
  m_audio.Shutdown();
  m_editor.Shutdown();
    
    if (gLua) { gLua->Shutdown(); gLua.reset(); }
  m_renderingSystem.Shutdown();
  if (gSystemManager)
  {
    gSystemManager->textRenderingSystem.Shutdown();
    gSystemManager.reset();
  }

#ifdef AARTZE_WITH_IMGUI
  if (m_imguiReady)
  {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    m_imguiReady = false;
  }
#endif
  if (m_window)
  {
    glfwDestroyWindow(m_window);
    glfwTerminate();
  }
}

void Application::Run()
{
    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(m_window))
    {
        gProfiler.NewFrame();
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        glfwPollEvents();
        m_inputHandler.Poll();

        // Drive rendering from editor shell; keep camera speed stable
        m_renderingSystem.SetCameraSpeed(2.0f);
        if (gSystemManager)
            gSystemManager->Update(deltaTime);
        {
            ProfileScope _p(gProfiler, "Render");
            m_renderingSystem.Render();
        }
        int w, h;
        glfwGetFramebufferSize(m_window, &w, &h);
    m_editor.Resize(w, h);
    m_editor.Tick(deltaTime);

    if (gSystemManager)
        gSystemManager->textRenderingSystem.Render(w, h);

#ifdef AARTZE_WITH_IMGUI
    if (m_imguiReady)
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        m_aartzeUI.Draw();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
#else
    // Render UE-like editor shell (non-modal)
    m_editor.Render();
#endif

    glfwSwapBuffers(m_window);
    }
}





