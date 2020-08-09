#ifndef UTILS_CAMERA_H_
#define UTILS_CAMERA_H_

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace utils {

class Camera {
public:
  Camera(GLFWwindow* window);

  void LookAt(const glm::vec3& camera_pos, const glm::vec3& center);

  const glm::mat4& GetViewMatrix() const { return view_mat_; }

  void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
  GLFWwindow* glfw_window_;
  glm::mat4 view_mat_;
};

} // namespace utils

#endif // UTILS_CAMERA_H_