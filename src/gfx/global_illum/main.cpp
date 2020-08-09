#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/gl.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <unordered_map>
#include "utils/image.h"
#include "utils/model.h"
#include "utils/shader.h"
#include "utils/program.h"

constexpr int kWindowWidth = 1920;
constexpr int kWindowHeight = 1080;
const char* kWindowTitle = "Global Illum";

constexpr float kAspectRatio = static_cast<float>(kWindowWidth) / static_cast<float>(kWindowHeight);

GLuint gl_program;
GLuint gl_vao;

std::shared_ptr<utils::Model> model;
std::vector<GLuint> gl_pos_vbos;
std::vector<GLuint> gl_normal_vbos;
std::vector<GLuint> gl_texcoord_vbos;
std::vector<GLuint> gl_mtl_id_vbos;

std::unordered_map<std::string, std::shared_ptr<utils::Image>> tex_images;
std::unordered_map<std::string, int> texname_to_gl_tex;

void Initialize() {
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.f, 0.f, 0.f, 1.f);

  gl_program = glCreateProgram();
  if (!gl_program) {
    std::cerr << "Could not create gl_program." << std::endl;
    exit(1);
  }

  GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
  if (auto src_opt = utils::LoadShaderSource("geom_pass.vert")) {
    if (!utils::CompileShader(vert_shader, src_opt.value())) {
      std::cerr << "Could not compile vertex shader." << std::endl;
      exit(1);
    }
  } else {
    std:: cerr << "Could not load shader from file triangle.vert." << std::endl;
    exit(1);
  }

  GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
  if (auto src_opt = utils::LoadShaderSource("geom_pass.frag")) {
    if (!utils::CompileShader(frag_shader, src_opt.value())) {
      std::cerr << "Could not compile fragment shader." << std::endl;
      exit(1);
    }
  } else {
    std:: cerr << "Could not load shader from file triangle.frag." << std::endl;
    exit(1);
  }

  glAttachShader(gl_program, vert_shader);
  glAttachShader(gl_program, frag_shader);

  glLinkProgram(gl_program);
  if (!utils::CheckProgramLinkStatus(gl_program)) {
    exit(1);
  }
  
  glDeleteShader(frag_shader);
  glDeleteShader(vert_shader);

  glGenVertexArrays(1, &gl_vao);

  glUseProgram(gl_program);

  // Translates the model first so that its origin is near its center rather than its base.
  glm::mat4 model_mat = glm::mat4(1.f);
  glm::mat4 view_mat = glm::mat4(1.f);
  glm::mat4 proj_mat = glm::perspective(glm::radians(75.f), kAspectRatio, 0.1f, 1000.f);
  glm::mat4 mvp_mat = proj_mat * view_mat * model_mat;
    
  GLint mvp_mat_loc = glGetUniformLocation(gl_program, "mvp_mat");
  glUniformMatrix4fv(mvp_mat_loc, 1, GL_FALSE, glm::value_ptr(mvp_mat));

  model = utils::LoadModelFromFile("assets/sponza/sponza.obj", "assets/sponza");
  if (model == nullptr) {
    std::cerr << "Could not load model." << std::endl;
    exit(1);
  }

  for (const utils::Mesh& mesh : model->meshes) {
    for (const utils::Material& mtl : mesh.materials) {
      for (const std::string& texname : { mtl.ambient_texname, 
                                          mtl.diffuse_texname, 
                                          mtl.specular_texname }) {
        // Loads the image file if it hasn't been loaded in before.
        if (!texname.empty() && tex_images.find(texname) == tex_images.end()) {
          std::shared_ptr<utils::Image> img =
              utils::LoadImageFromFile("assets/sponza/" + texname, true);
          if (img != nullptr) {
            tex_images[texname] = std::move(img);
          } else {
            std::cerr << "Could not find image file: " << texname << std::endl;
          }
        }
      }
    }
  }

  gl_pos_vbos.resize(model->meshes.size());
  glGenBuffers(gl_pos_vbos.size(), &gl_pos_vbos[0]);
  for (size_t i = 0; i < gl_pos_vbos.size(); ++i) {
    glBindBuffer(GL_ARRAY_BUFFER, gl_pos_vbos[i]);
    glBufferData(GL_ARRAY_BUFFER, model->meshes[i].positions.size() * sizeof(glm::vec3), 
                 glm::value_ptr(model->meshes[i].positions[0]), GL_STATIC_DRAW);
  }

  gl_normal_vbos.resize(model->meshes.size());
  glGenBuffers(gl_normal_vbos.size(), &gl_normal_vbos[0]);
  for (size_t i = 0; i < gl_normal_vbos.size(); ++i) {
    glBindBuffer(GL_ARRAY_BUFFER, gl_normal_vbos[i]);
    glBufferData(GL_ARRAY_BUFFER, model->meshes[i].normals.size() * sizeof(glm::vec3), 
                 glm::value_ptr(model->meshes[i].normals[0]), GL_STATIC_DRAW);
  }

  gl_texcoord_vbos.resize(model->meshes.size());
  glGenBuffers(gl_texcoord_vbos.size(), &gl_texcoord_vbos[0]);
  for (size_t i = 0; i < gl_texcoord_vbos.size(); ++i) {
    glBindBuffer(GL_ARRAY_BUFFER, gl_texcoord_vbos[i]);
    glBufferData(GL_ARRAY_BUFFER, model->meshes[i].texcoords.size() * sizeof(glm::vec2), 
                 glm::value_ptr(model->meshes[i].texcoords[0]), GL_STATIC_DRAW);
  }

  gl_mtl_id_vbos.resize(model->meshes.size());
  glGenBuffers(gl_mtl_id_vbos.size(), &gl_mtl_id_vbos[0]);
  for (size_t i = 0; i < gl_mtl_id_vbos.size(); ++i) {
    glBindBuffer(GL_ARRAY_BUFFER, gl_mtl_id_vbos[i]);
    glBufferData(GL_ARRAY_BUFFER, model->meshes[i].material_ids.size() * sizeof(int),
                 &model->meshes[i].material_ids[0], GL_STATIC_DRAW);
  }
}

void RenderPass() {
  glViewport(0, 0, kWindowWidth, kWindowHeight);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(gl_program);
  glBindVertexArray(gl_vao);

  for (size_t i = 0; i < model->meshes.size(); ++i) {
    glBindBuffer(GL_ARRAY_BUFFER, gl_pos_vbos[i]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, gl_normal_vbos[i]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, gl_texcoord_vbos[i]);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLES, 0, model->meshes[i].num_verts);
  }
}

void Cleanup() {
  glDeleteBuffers(gl_mtl_id_vbos.size(), &gl_mtl_id_vbos[0]);
  glDeleteBuffers(gl_texcoord_vbos.size(), &gl_texcoord_vbos[0]);
  glDeleteBuffers(gl_normal_vbos.size(), &gl_normal_vbos[0]);
  glDeleteBuffers(gl_pos_vbos.size(), &gl_pos_vbos[0]);
  glDeleteVertexArrays(1, &gl_vao);
  glDeleteProgram(gl_program);
}

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
  
  GLFWwindow* glfw_window = glfwCreateWindow(kWindowWidth, kWindowHeight, kWindowTitle, nullptr, 
                                             nullptr);
  if (glfw_window == nullptr) {
    std::cerr << "Could not create GLFW window." << std::endl;
    exit(1);
  }

  glfwMakeContextCurrent(glfw_window);

  glewExperimental = true;
  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW" << std::endl;
    exit(1);
  }

  Initialize();

  while (!glfwWindowShouldClose(glfw_window)) {
    glfwPollEvents();
    RenderPass();
    glfwSwapBuffers(glfw_window);
  }

  Cleanup();

  if (glfw_window != nullptr) {
    glfwDestroyWindow(glfw_window);
  }
  glfwTerminate();

  return 0;
}