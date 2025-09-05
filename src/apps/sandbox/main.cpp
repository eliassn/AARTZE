#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <cstdio>

static void errorcb(int code, const char* desc){ std::fprintf(stderr, "GLFW error %d: %s\n", code, desc); }

int main(){
  glfwSetErrorCallback(errorcb);
  if(!glfwInit()) return -1;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  auto* win = glfwCreateWindow(1280, 720, "AARTZE Sandbox", nullptr, nullptr);
  if(!win){ glfwTerminate(); return -1; }
  glfwMakeContextCurrent(win);
  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
    std::fprintf(stderr, "Failed to load GL functions\n");
    return -1;
  }
  std::printf("GPU: %s\n", glGetString(GL_RENDERER));
  glClearColor(0.05f,0.05f,0.07f,1.0f);
  while(!glfwWindowShouldClose(win)){
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    // TODO: draw grid + cube
    glfwSwapBuffers(win);
    glfwPollEvents();
  }
  glfwDestroyWindow(win);
  glfwTerminate();
  return 0;
}

