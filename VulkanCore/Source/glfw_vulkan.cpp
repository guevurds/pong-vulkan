#include <vulkan/vulkan.h>

#include "my_vulkan_glfw.h"

namespace MyVK{
  static void GLFW_KeyCallbacks(GLFWwindow* pWindow, int Key, int Scancode, int Action, int Mods) {
    GLFWCallbacks* pGLFWCallbacks = (GLFWCallbacks*)glfwGetWindowUserPointer(pWindow);
    pGLFWCallbacks->Key(pWindow, Key, Scancode, Action, Mods);

  }

  static void GLFW_MouseCallback(GLFWwindow* pWindow, double xpos, double ypos) {
    GLFWCallbacks* pGLFWCallbacks = (GLFWCallbacks*)glfwGetWindowUserPointer(pWindow);
    pGLFWCallbacks->MouseMove(pWindow, xpos, ypos);
  }
  
  static void GLFW_MouseButtonCallback(GLFWwindow* pWindow, int Button, int Action, int Mods) {
    GLFWCallbacks* pGLFWCallbacks = (GLFWCallbacks*)glfwGetWindowUserPointer(pWindow);
    pGLFWCallbacks->MouseButton(pWindow, Button, Action, Mods);
  }

  GLFWwindow* glfw_vulkan_init(int Width, int Height, const char* pTitle) {
    if (!glfwInit()) {
      exit(EXIT_FAILURE);
    }

    if(!glfwVulkanSupported()) {
      exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, 0);

    GLFWwindow* pWindow = glfwCreateWindow(Width, Height, pTitle, NULL, NULL);

    if (!pWindow) {
      glfwTerminate();
      exit(EXIT_FAILURE);
    }

    return pWindow;
  }

  void glfw_vulkan_set_callbacks(GLFWwindow* pWindow, GLFWCallbacks* pCallbacks) {
    glfwSetWindowUserPointer(pWindow, pCallbacks);

    glfwSetKeyCallback(pWindow, GLFW_KeyCallbacks);
    glfwSetCursorPosCallback(pWindow, GLFW_MouseCallback);
    glfwSetMouseButtonCallback(pWindow, GLFW_MouseButtonCallback);
  }
}