#ifndef UTILS_CAMERA_H_
#define UTILS_CAMERA_H_

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <limits>

namespace utils {

class Camera {
public:
  Camera(GLFWwindow* window);

  void Tick();

  void SetCameraPos(const glm::vec3 camera_pos);

  const glm::mat4& GetViewMatrix() const { return view_mat_; }
  const glm::vec3& GetCameraPos() const { return camera_pos_; }

  void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
  void MouseCallback(GLFWwindow* window, double x, double y);

private:
  glm::vec3 LocalToGlobalTransform(glm::vec3 vec_local);

private:
  GLFWwindow* glfw_window_;

  glm::mat4 view_mat_;
  glm::quat camera_rotation_;
  glm::vec3 camera_pos_;

  bool fps_mode_ = false;

  bool move_forward_ = false;
  bool move_backward_ = false;
  bool move_left_ = false;
  bool move_right_ = false;

  const double kMouseNoValue = std::numeric_limits<double>::max();
  double prev_mouse_x_ = kMouseNoValue;
  double prev_mouse_y_ = kMouseNoValue;
};

} // namespace utils

#endif // UTILS_CAMERA_H_