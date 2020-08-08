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
#include "utils/image.h"
#include "utils/model.h"
#include "utils/shader.h"
#include "utils/program.h"

constexpr int kWindowWidth = 1920;
constexpr int kWindowHeight = 1080;
const char* kWindowTitle = "Local Illum";

constexpr float kAspectRatio = static_cast<float>(kWindowWidth) / static_cast<float>(kWindowHeight);

GLuint gl_program;
GLuint gl_vao;
GLuint gl_pos_vbo;
GLuint gl_normal_vbo;
GLuint gl_texcoord_vbo;
GLuint gl_texture;
std::shared_ptr<utils::Model> model;
std::shared_ptr<utils::Image> img;

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
  if (auto src_opt = utils::LoadShaderSource("model.vert")) {
    if (!utils::CompileShader(vert_shader, src_opt.value())) {
      std::cerr << "Could not compile vertex shader." << std::endl;
      exit(1);
    }
  } else {
    std:: cerr << "Could not load shader from file triangle.vert." << std::endl;
    exit(1);
  }

  GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
  if (auto src_opt = utils::LoadShaderSource("model.frag")) {
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
  glm::mat4 model_mat = 
      glm::rotate(glm::mat4(1.f), -(glm::pi<float>() / 2.f), glm::vec3(1.f, 0.f, 0.f)) *
      glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -6.5f));
  glm::mat4 view_mat = glm::lookAt(glm::vec3(0.f, 15.f, 22.f), glm::vec3(0.f, 0.f, 0.f),
                                   glm::vec3(0.f, 1.f, 0.f));
  glm::mat4 proj_mat = glm::perspective(glm::radians(75.f), kAspectRatio, 0.1f, 
                                1000.f);
  glm::mat4 mvp_mat = proj_mat * view_mat * model_mat;

  GLint mvp_mat_loc = glGetUniformLocation(gl_program, "mvp_mat");
  glUniformMatrix4fv(mvp_mat_loc, 1, GL_FALSE, glm::value_ptr(mvp_mat));

   img = utils::LoadImageFromFile("assets/teapot/texture.jpg", true /* flip */);
  if (img == nullptr) {
    std::cerr << "Could not load image." << std::endl;
    exit(1);
  }
  assert(img->format == utils::ImageFormat::kRGB);

  glGenTextures(1, &gl_texture);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, gl_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->width, img->height, 0, GL_RGB, GL_UNSIGNED_BYTE, 
               img->data.data());

  GLint sampler_loc = glGetUniformLocation(gl_program, "tex_sampler");
  glUniform1i(sampler_loc, 1);

  model = utils::LoadModelFromFile("assets/teapot/teapot.obj", "assets/teapot");
  if (model == nullptr) {
    std::cerr << "Could not load model." << std::endl;
    exit(1);
  }

  glGenBuffers(1, &gl_pos_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, gl_pos_vbo);
  glBufferData(GL_ARRAY_BUFFER, model->positions.size() * 3, glm::value_ptr(model->positions[0]), 
               GL_STATIC_DRAW);

  glGenBuffers(1, &gl_normal_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, gl_normal_vbo);
  glBufferData(GL_ARRAY_BUFFER, model->normals.size() * 3, glm::value_ptr(model->normals[0]), 
               GL_STATIC_DRAW);

  glGenBuffers(1, &gl_texcoord_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, gl_texcoord_vbo);
  glBufferData(GL_ARRAY_BUFFER, model->texcoords.size() * 2, glm::value_ptr(model->texcoords[0]), 
               GL_STATIC_DRAW);
}

void RenderPass() {
  glViewport(0, 0, kWindowWidth, kWindowHeight);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(gl_program);
  glBindVertexArray(gl_vao);

  glBindBuffer(GL_ARRAY_BUFFER, gl_pos_vbo);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, gl_normal_vbo);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, gl_texcoord_vbo);
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

  glDrawArrays(GL_TRIANGLES, 0, model->num_verts);
}

void Cleanup() {
  glDeleteBuffers(1, &gl_texcoord_vbo);
  glDeleteBuffers(1, &gl_normal_vbo);
  glDeleteBuffers(1, &gl_pos_vbo);
  glDeleteTextures(1, &gl_texture);
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