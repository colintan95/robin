#include "utils/camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include <iostream>

namespace utils {

namespace {

const float kWalkSpeed = 0.2f;
const float kStrafeSpeed = 0.1f;

// TODO: See if there's a better way to pass the member method Camera::KeyCallback() into
// glfwSetKeyCallback(). Right now, glfwSetKeyCallback() can't take in a member method and so need
// we need a static wrapper that will call our member method.

static Camera* camera = nullptr;

static void KeyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods) {
  return camera->KeyCallback(window, key, scancode, action, mods);
}

} // internal

Camera::Camera(GLFWwindow* window) 
    : glfw_window_(window), view_mat_(1.f), camera_rotation_(1.f, 0.f, 0.f, 0.f) {
  if (glfw_window_ == nullptr) {
    throw;
  }
  camera = this;

  glfwSetKeyCallback(window, KeyCallbackWrapper);
}

void Camera::Tick() {
  float walk_speed = kWalkSpeed;
  float strafe_speed = kStrafeSpeed;

  if (move_forward_) {
    camera_pos_ += LocalToGlobalTransform(glm::vec3(0.f, 0.f, -walk_speed));
  }
  if (move_backward_) {
    camera_pos_ += LocalToGlobalTransform(glm::vec3(0.f, 0.f, walk_speed));
  }
  if (move_left_) {
    camera_pos_ += LocalToGlobalTransform(glm::vec3(-strafe_speed, 0.f, 0.f));
  }
  if (move_right_) {
    camera_pos_ += LocalToGlobalTransform(glm::vec3(strafe_speed, 0.f, 0.f));
  }

  // TODO: Is the inverse operation expensive?
  view_mat_ = // glm::inverse(glm::mat4_cast(camera_rotation_)) *
              glm::translate(glm::mat4(1.f), -camera_pos_);
}

void Camera::SetCameraPos(const glm::vec3 camera_pos) {
  camera_pos_ = camera_pos;
}

void Camera::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_W:
        move_forward_ = true;
        break;
      case GLFW_KEY_S:
        move_backward_ = true;
        break;
      case GLFW_KEY_A:
        move_left_ = true;
        break;
      case GLFW_KEY_D:
        move_right_ = true;
        break;
    }
  } else if (action == GLFW_RELEASE) {
    switch (key) {
      case GLFW_KEY_W:
        move_forward_ = false;
        break;
      case GLFW_KEY_S:
        move_backward_ = false;
        break;
      case GLFW_KEY_A:
        move_left_ = false;
        break;
      case GLFW_KEY_D:
        move_right_ = false;
        break;
    }
  }
}

glm::vec3 Camera::LocalToGlobalTransform(glm::vec3 vec_local) {
  return glm::mat3_cast(camera_rotation_) * vec_local;
}

} // namespace utils