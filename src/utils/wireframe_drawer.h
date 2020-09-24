#ifndef UTILS_WIREFRAME_DRAWER_H_
#define UTILS_WIREFRAME_DRAWER_H_

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>

#include <vector>

namespace utils {

namespace {

}  // namespace

class WireframeDrawer {
public:
  WireframeDrawer();
  ~WireframeDrawer();

  void Draw(glm::mat4 pv_mat);

  void AddRectangle(glm::vec3 center, float width, float height, float depth);

private:
  struct WireframeMesh {
    GLuint gl_vbo;
    int num_triangles;
  };

  GLuint gl_program_;
  GLuint gl_vao_;
  std::vector<WireframeMesh> meshes_;
};

}  // namespace utils

#endif  // UTILS_WIREFRAME_DRAWER_H_