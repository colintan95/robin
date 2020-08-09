#include "utils/camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

namespace utils {

namespace {

// TODO: See if there's a better way to pass the member method Camera::KeyCallback() into
// glfwSetKeyCallback(). Right now, glfwSetKeyCallback() can't take in a member method and so need
// we need a static wrapper that will call our member method.

static Camera* camera = nullptr;

static void KeyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods) {
  return camera->KeyCallback(window, key, scancode, action, mods);
}

} // internal

Camera::Camera(GLFWwindow* window) : glfw_window_(window), view_mat_(1.f) {
  if (glfw_window_ == nullptr) {
    throw;
  }
  camera = this;

  glfwSetKeyCallback(window, KeyCallbackWrapper);
}

void Camera::LookAt(const glm::vec3& camera_pos, const glm::vec3& center) {
  view_mat_ = glm::lookAt(camera_pos, center, glm::vec3(0.f, 1.f, 0.f));
}

void Camera::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  std::cout << "GLFW key pressed" << std::endl;
}

} // namespace utils