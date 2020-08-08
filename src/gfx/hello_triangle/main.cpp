#include <iostream>
#include <cstdlib>

#include <GLFW/glfw3.h>

int main() {
  if (!glfwInit()) {
    std::cerr << "Could not initialize GLFW." << std::endl;
    exit(1);
  }

  std::cout << "Hello, World!" << std::endl;

  glfwTerminate();

  return 0;
}