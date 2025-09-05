#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "AartzeEditor.h"
#include "Renderer.h"

using namespace aartze;

static void glfw_error_cb(int err, const char* desc){ (void)err; fprintf(stderr, "GLFW error: %s\n", desc); }

int main(){
    glfwSetErrorCallback(glfw_error_cb);
    if(!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* win = glfwCreateWindow(1280, 720, "AARTZE ImGui Demo", nullptr, nullptr);
    if(!win){ glfwTerminate(); return 1; }
    glfwMakeContextCurrent(win);
    glfwSwapInterval(1);
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return 2;

    // ImGui + docking
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // Enable docking when available (builds with or without docking branch)
#ifdef IMGUI_HAS_DOCK
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#endif
    ImGui_ImplGlfw_InitForOpenGL(win, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Editor + renderer
    EditorUI ui; EditorUI::ApplyTheme(); ui.StartSplash();
    Renderer renderer; renderer.initGL(); ui.SetRenderer(&renderer);

    while(!glfwWindowShouldClose(win)){
        glfwPollEvents();
        int fbw, fbh; glfwGetFramebufferSize(win,&fbw,&fbh);
        glViewport(0,0,fbw,fbh); glClearColor(0.18f,0.19f,0.22f,1.0f); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Fullscreen dockspace (only when docking branch is used)
#ifdef IMGUI_HAS_DOCK
        {
            ImGuiWindowFlags dockFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                         ImGuiWindowFlags_NoBringToFrontOnFocus |
                                         ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoDecoration |
                                         ImGuiWindowFlags_NoSavedSettings;
            ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->Pos);
            ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
            ImGui::Begin("##DockHost", nullptr, dockFlags);
            ImGui::DockSpace(ImGui::GetID("AARTZE_DockSpace"), ImVec2(0,0), ImGuiDockNodeFlags_PassthruCentralNode);
            ImGui::End();
            ImGui::PopStyleVar();
        }
#endif

        ui.Draw();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(win);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
