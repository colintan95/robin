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
#include "utils/camera.h"
#include "utils/image.h"
#include "utils/model.h"
#include "utils/shader.h"
#include "utils/program.h"

constexpr int kWindowWidth = 1920;
constexpr int kWindowHeight = 1080;
const char* kWindowTitle = "Local Illum";

constexpr float kAspectRatio = static_cast<float>(kWindowWidth) / static_cast<float>(kWindowHeight);

const int kShadowTexWidth = 1024;
const int kShadowTexHeight = 1024;

std::unique_ptr<utils::Camera> camera;

GLuint gl_program;
GLuint gl_vao;
std::vector<GLuint> gl_pos_vbos;
std::vector<GLuint> gl_normal_vbos;
std::shared_ptr<utils::Model> model;

GLuint gl_shadow_program;
GLuint gl_shadow_vao;
GLuint gl_shadow_fbo;
GLuint gl_shadow_tex;

glm::vec3 light_pos;

void Initialize() {
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.f, 0.f, 0.f, 1.f);

  model = utils::LoadModelFromFile("assets/cornell_box/cornell_box.obj", "assets/cornell_box");
  if (model == nullptr) {
    std::cerr << "Could not load model." << std::endl;
    exit(1);
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

  light_pos = glm::vec3(0.f, 9.0f, 0.f);
}

void CreateShadowPass() {
  gl_shadow_program = glCreateProgram();
  if (!gl_shadow_program) {
    std::cerr << "Could not create program." << std::endl;
    exit(1);
  }

  GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
  if (auto src_opt = utils::LoadShaderSource("shadow_pass.vert")) {
    if (!utils::CompileShader(vert_shader, src_opt.value())) {
      std::cerr << "Could not compile vertex shader." << std::endl;
      exit(1);
    }
  } else {
    std:: cerr << "Could not load shader from file triangle.vert." << std::endl;
    exit(1);
  }

  GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
  if (auto src_opt = utils::LoadShaderSource("shadow_pass.frag")) {
    if (!utils::CompileShader(frag_shader, src_opt.value())) {
      std::cerr << "Could not compile fragment shader." << std::endl;
      exit(1);
    }
  } else {
    std:: cerr << "Could not load shader from file triangle.frag." << std::endl;
    exit(1);
  }

  glAttachShader(gl_shadow_program, vert_shader);
  glAttachShader(gl_shadow_program, frag_shader);

  glLinkProgram(gl_shadow_program);
  if (!utils::CheckProgramLinkStatus(gl_shadow_program)) {
    exit(1);
  }
  
  glDeleteShader(frag_shader);
  glDeleteShader(vert_shader);

  glGenVertexArrays(1, &gl_shadow_vao);

  glGenTextures(1, &gl_shadow_tex);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, gl_shadow_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, kShadowTexWidth, kShadowTexHeight, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glGenFramebuffers(1, &gl_shadow_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, gl_shadow_fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gl_shadow_tex, 0);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CreateLightPass() {
  gl_program = glCreateProgram();
  if (!gl_program) {
    std::cerr << "Could not create gl_program." << std::endl;
    exit(1);
  }

  GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
  if (auto src_opt = utils::LoadShaderSource("local_illum.vert")) {
    if (!utils::CompileShader(vert_shader, src_opt.value())) {
      std::cerr << "Could not compile vertex shader." << std::endl;
      exit(1);
    }
  } else {
    std:: cerr << "Could not load shader from file triangle.vert." << std::endl;
    exit(1);
  }

  GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
  if (auto src_opt = utils::LoadShaderSource("local_illum.frag")) {
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

  glm::vec3 light_pos = glm::vec3(0.f, 9.0f, 0.f);
  glm::vec3 ambient_I = glm::vec3(0.8f, 0.8f, 0.8f);
  glm::vec3 diffuse_I = glm::vec3(0.3f, 0.3f, 0.3f);
  glm::vec3 specular_I = glm::vec3(1.f, 1.f, 1.f);
  float shininess = 8.f;

  camera->SetCameraPos(glm::vec3(0.f, 5.f, 12.5f));

  GLint light_pos_loc = glGetUniformLocation(gl_program, "light_pos");
  glUniform3fv(light_pos_loc, 1, glm::value_ptr(light_pos));

  GLint ambient_loc = glGetUniformLocation(gl_program, "ambient_I");
  glUniform3fv(ambient_loc, 1, glm::value_ptr(ambient_I));

  GLint diffuse_loc = glGetUniformLocation(gl_program, "diffuse_I");
  glUniform3fv(diffuse_loc, 1, glm::value_ptr(diffuse_I));

  GLint specular_loc = glGetUniformLocation(gl_program, "specular_I");
  glUniform3fv(specular_loc, 1, glm::value_ptr(specular_I));
}

void ShadowPass() {
  glBindFramebuffer(GL_FRAMEBUFFER, gl_shadow_fbo);

  glViewport(0, 0, kShadowTexWidth, kShadowTexHeight);
  glClear(GL_DEPTH_BUFFER_BIT);

  glUseProgram(gl_shadow_program);

  for (size_t i = 0; i < model->meshes.size(); ++i) {
    const utils::Mesh& mesh = model->meshes[i];

    glm::mat4 model_mat = glm::scale(glm::mat4(1.f), glm::vec3(5.f, 5.f, 5.f));
    glm::mat4 view_mat = glm::lookAt(light_pos, light_pos + glm::vec3(0.f, -1.f, 0.f),
                                     glm::vec3(0.f, 0.f, 1.f));
    glm::mat4 proj_mat = glm::perspective(glm::radians(90.f), 1.f, 1.f, 30.f);

    glm::mat4 mvp_mat = proj_mat * view_mat * model_mat;

    GLint mvp_mat_loc = glGetUniformLocation(gl_shadow_program, "mvp_mat");
    glUniformMatrix4fv(mvp_mat_loc, 1, GL_FALSE, glm::value_ptr(mvp_mat));

    glBindVertexArray(gl_shadow_vao);

    glBindBuffer(GL_ARRAY_BUFFER, gl_pos_vbos[i]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLES, 0, mesh.num_verts);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void LightPass() {
  glViewport(0, 0, kWindowWidth, kWindowHeight);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(gl_program);

  for (size_t i = 0; i < model->meshes.size(); ++i) {
    const utils::Mesh& mesh = model->meshes[i];

    glm::mat4 model_mat = glm::scale(glm::mat4(1.f), glm::vec3(5.f, 5.f, 5.f));
    glm::mat4 view_mat = camera->GetViewMatrix();
    glm::mat4 proj_mat = glm::perspective(glm::radians(75.f), kAspectRatio, 0.1f, 1000.f);

    glm::mat4 mv_mat = view_mat * model_mat;
    glm::mat4 mvp_mat = proj_mat * mv_mat;

    GLint model_mat_loc = glGetUniformLocation(gl_program, "model_mat");
    glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, glm::value_ptr(model_mat));
      
    GLint mvp_mat_loc = glGetUniformLocation(gl_program, "mvp_mat");
    glUniformMatrix4fv(mvp_mat_loc, 1, GL_FALSE, glm::value_ptr(mvp_mat));

    glm::mat3 normal_mat = glm::transpose(glm::inverse(glm::mat3(mv_mat)));
    GLint normal_mat_loc = glGetUniformLocation(gl_program, "normal_mat");
    glUniformMatrix3fv(normal_mat_loc, 1, GL_FALSE, glm::value_ptr(normal_mat)); 

    GLint camera_pos_loc = glGetUniformLocation(gl_program, "camera_pos");
    glUniform3fv(camera_pos_loc, 1, glm::value_ptr(camera->GetCameraPos()));

    GLint ambient_color_loc = glGetUniformLocation(gl_program, "ambient_color");
    glUniform3fv(ambient_color_loc, 1, glm::value_ptr(mesh.materials[0].ambient_color));

    GLint specular_color_loc = glGetUniformLocation(gl_program, "specular_color");
    glUniform3fv(specular_color_loc, 1, glm::value_ptr(mesh.materials[0].specular_color));

    GLint shininess_loc = glGetUniformLocation(gl_program, "shininess");
    glUniform1f(shininess_loc, mesh.materials[0].shininess);

    glBindVertexArray(gl_vao);

    glBindBuffer(GL_ARRAY_BUFFER, gl_pos_vbos[i]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, gl_normal_vbos[i]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLES, 0, mesh.num_verts);
  }
}

void Cleanup() {
  glDeleteFramebuffers(1, &gl_shadow_fbo);
  glDeleteTextures(1, &gl_shadow_tex);
  glDeleteVertexArrays(1, &gl_shadow_vao);
  glDeleteProgram(gl_shadow_program);

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

  camera = std::make_unique<utils::Camera>(glfw_window);

  Initialize();
  CreateShadowPass();
  CreateLightPass();

  while (!glfwWindowShouldClose(glfw_window)) {
    glfwPollEvents();
    camera->Tick();

    ShadowPass();
    LightPass();

    glfwSwapBuffers(glfw_window);
  }

  Cleanup();

  camera.reset();

  if (glfw_window != nullptr) {
    glfwDestroyWindow(glfw_window);
  }
  glfwTerminate();

  return 0;
}