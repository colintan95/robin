#include "utils/wireframe_drawer.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>

#include "utils/program.h"
#include "utils/shader.h"

namespace utils {

namespace {
   
const char kVertShaderSource[] = 
    "#version 430 core\n"
    "in vec3 vert_pos;\n"
    "uniform mat4 vp_mat;\n"
    "void main() {\n"
    "  gl_Position = vp_mat * vec4(vert_pos, 1.0);\n"
    "}";

const char kFragShaderSource[] = 
    "#version 430 core\n"
    "out vec4 out_color;\n"
    "void main() {\n"
    "  out_color = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "}";

}  // namespace

WireframeDrawer::WireframeDrawer() {
  gl_program_ = glCreateProgram();

  GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
  // TODO: Use string view.
  if (!utils::CompileShader(vert_shader, std::string(kVertShaderSource))) {
    // TODO: Do something better.
    throw;
  }

  GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    // TODO: Use string view.
  if (!utils::CompileShader(frag_shader, std::string(kFragShaderSource))) {
    throw;
  }

  glAttachShader(gl_program_, vert_shader);
  glAttachShader(gl_program_, frag_shader);

  glLinkProgram(gl_program_);
  if (!utils::CheckProgramLinkStatus(gl_program_)) {
    throw;
  }

  glDeleteShader(frag_shader);
  glDeleteShader(vert_shader);

  glGenVertexArrays(1, &gl_vao_);
}

WireframeDrawer::~WireframeDrawer() {
  for (const WireframeMesh& mesh : meshes_) {
    glDeleteBuffers(1, &mesh.gl_vbo);
  }
  meshes_.clear();

  glDeleteVertexArrays(1, &gl_vao_);
  glDeleteProgram(gl_program_);
}

void WireframeDrawer::Draw(glm::mat4 vp_mat) {
  glUseProgram(gl_program_);

  GLint vp_mat_loc = glGetUniformLocation(gl_program_, "vp_mat");
  glUniformMatrix4fv(vp_mat_loc, 1, GL_FALSE, glm::value_ptr(vp_mat));

  glBindVertexArray(gl_vao_);

  for (const WireframeMesh& mesh : meshes_) {
    glBindBuffer(GL_ARRAY_BUFFER, mesh.gl_vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_LINE_STRIP, 0, mesh.num_triangles * 3);
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glUseProgram(0);
}

void WireframeDrawer::AddRectangle(glm::vec3 center, float width, float height, float depth) {
  WireframeMesh mesh;

  float hw = width / 2.f;
  float hh = height / 2.f;
  float hd = depth / 2.f;

  float pos_x = center.x + hw;
  float neg_x = center.x - hw;
  float pos_y = center.y + hh;
  float neg_y = center.y - hh;
  float pos_z = center.z + hd;
  float neg_z = center.z - hd;

  const glm::vec3 pos_data[] = {
    // +x
    {pos_x, pos_y, pos_z}, {pos_x, neg_y, pos_z}, {pos_x, neg_y, neg_z}, 
    {pos_x, pos_y, pos_z}, {pos_x, neg_y, neg_z}, {pos_x, pos_y, neg_z},
    // -x
    {neg_x, pos_y, neg_z}, {neg_x, neg_y, neg_z}, {neg_x, neg_y, pos_z}, 
    {neg_x, pos_y, neg_z}, {neg_x, neg_y, pos_z}, {neg_x, pos_y, pos_z},
    // +y
    {pos_x, pos_y, pos_z}, {pos_x, pos_y, neg_z}, {neg_x, pos_y, neg_z},
    {pos_x, pos_y, pos_z}, {neg_x, pos_y, neg_z}, {neg_x, pos_y, pos_z},
    // -y
    {neg_x, neg_y, pos_z}, {neg_x, neg_y, neg_z}, {pos_x, neg_y, neg_z},
    {neg_x, neg_y, pos_z}, {pos_x, neg_y, neg_z}, {pos_x, neg_y, pos_z},
    // +z
    {neg_x, pos_y, pos_z}, {neg_x, neg_y, pos_z}, {pos_x, neg_y, pos_z},
    {neg_x, pos_y, pos_z}, {pos_x, neg_y, pos_z}, {pos_x, pos_y, pos_z},
    // -z
    {pos_x, pos_y, neg_z}, {pos_x, neg_y, neg_z}, {neg_x, neg_y, neg_z},
    {pos_x, pos_y, neg_z}, {neg_x, neg_y, neg_z}, {neg_x, pos_y, neg_z}
  };

  glGenBuffers(1, &mesh.gl_vbo);

  glBindBuffer(GL_ARRAY_BUFFER, mesh.gl_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(pos_data), glm::value_ptr(pos_data[0]), GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  mesh.num_triangles = 12;

  meshes_.push_back(mesh);
}

}  // namespace utils