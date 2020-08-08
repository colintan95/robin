#include <iostream>
#include <cstdlib>

#include <GL/glew.h>
#include <GL/gl.h>

#include <GLFW/glfw3.h>

void WindowErrorCallback(int error, const char* desc) {
  std::cerr << "GLFW Error: " << error << ": " << desc << std::endl;
}

int main() {
  if (!glfwInit()) {
    std::cerr << "Could not initialize GLFW." << std::endl;
    exit(1);
  }
  glfwSetErrorCallback(WindowErrorCallback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  
  GLFWwindow* glfw_window = glfwCreateWindow(1920, 1080, "Hello, Triangle!", nullptr, nullptr);
  if (glfw_window == nullptr) {
    std::cerr << "Could not create GLFW window." << std::endl;
    exit(1);
  }

  glfwMakeContextCurrent(glfw_window);

  while (!glfwWindowShouldClose(glfw_window)) {
    glfwPollEvents();

    glfwSwapBuffers(glfw_window);
  }

  if (glfw_window != nullptr) {
    glfwDestroyWindow(glfw_window);
  }
  glfwTerminate();

  return 0;
}