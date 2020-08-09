#include "utils/camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include <iostream>
#include <limits>

namespace utils {

namespace {
  
// TODO: See if there's a better way to pass the member method Camera::KeyCallback() into
// glfwSetKeyCallback(). Right now, glfwSetKeyCallback() can't take in a member method and so need
// we need a static wrapper that will call our member method.

static Camera* camera = nullptr;

static void KeyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods) {
  camera->KeyCallback(window, key, scancode, action, mods);
}

static void MouseCallbackWrapper(GLFWwindow *window, double x, double y) {
  camera->MouseCallback(window, x, y);
}

} // internal

Camera::Camera(GLFWwindow* window) 
    : glfw_window_(window), view_mat_(1.f), camera_rotation_(1.f, 0.f, 0.f, 0.f),
      camera_pos_(0.f, 0.f, 0.f) {
  if (glfw_window_ == nullptr) {
    throw;
  }
  camera = this;

  glfwSetKeyCallback(window, KeyCallbackWrapper);
  glfwSetCursorPosCallback(window, MouseCallbackWrapper);
}

void Camera::Tick() {
  if (fps_mode_) {
    if (move_forward_) {
      camera_pos_ += LocalToGlobalTransform(glm::vec3(0.f, 0.f, -walk_speed_));
    }
    if (move_backward_) {
      camera_pos_ += LocalToGlobalTransform(glm::vec3(0.f, 0.f, walk_speed_));
    }
    if (move_left_) {
      camera_pos_ += LocalToGlobalTransform(glm::vec3(-strafe_speed_, 0.f, 0.f));
    }
    if (move_right_) {
      camera_pos_ += LocalToGlobalTransform(glm::vec3(strafe_speed_, 0.f, 0.f));
    }
  }

  // TODO: Is the inverse operation expensive?
  view_mat_ = glm::inverse(glm::mat4_cast(camera_rotation_)) *
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
      case GLFW_KEY_Z:
        fps_mode_ = !fps_mode_;
        prev_mouse_x_ = kMouseNoValue;
        prev_mouse_y_ = kMouseNoValue;
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

void Camera::MouseCallback(GLFWwindow* window, double x, double y) {
  if (fps_mode_) {
    if (prev_mouse_x_ != kMouseNoValue && prev_mouse_y_ != kMouseNoValue) {
      float yaw_change = -static_cast<float>(x - prev_mouse_x_) * look_speed_;
      float pitch_change = -static_cast<float>(y - prev_mouse_y_) * look_speed_;

      // Yaw rotation is relative to the world coordinates.
      camera_rotation_ = 
          glm::rotate(glm::quat(1.f, 0.f, 0.f, 0.f), yaw_change, glm::vec3(0.f, 1.f, 0.f)) *
          camera_rotation_;
      
      // Pitch rotation is relative to the local coordinates of the camera.
      camera_rotation_ = glm::rotate(camera_rotation_, pitch_change, glm::vec3(1.f, 0.f, 0.f));
    }
    prev_mouse_x_ = x;
    prev_mouse_y_ = y;
  }
}

glm::vec3 Camera::LocalToGlobalTransform(glm::vec3 vec_local) {
  return glm::mat3_cast(camera_rotation_) * vec_local;
}

} // namespace utils