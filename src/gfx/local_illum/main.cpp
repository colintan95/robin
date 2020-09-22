#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/gl.h>


#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
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

constexpr int kShadowTexWidth = 1024;
constexpr int kShadowTexHeight = 1024;
constexpr float kShadowNearPlane = 0.5f;
constexpr float kShadowFarPlane = 30.f;

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
GLuint gl_shadow_rbo;

glm::vec3 light_pos;
glm::mat4 shadow_view_mats[6];
glm::mat4 shadow_proj_mat;

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

  light_pos = glm::vec3(0.f, 8.0f, 0.f);
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

  glUseProgram(gl_shadow_program);

  glGenVertexArrays(1, &gl_shadow_vao);

  glGenTextures(1, &gl_shadow_tex);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_CUBE_MAP, gl_shadow_tex);
  for (size_t i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, 
                 kShadowTexWidth, kShadowTexHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  shadow_view_mats[0] = // +x
      glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f)) *
      glm::translate(glm::mat4(1.f), -light_pos);
  shadow_view_mats[1] = // -x
      glm::rotate(glm::mat4(1.f), glm::radians(-90.f), glm::vec3(0.f, 1.f, 0.f)) *
      glm::translate(glm::mat4(1.f), -light_pos);
  shadow_view_mats[2] = // +y
      glm::rotate(glm::mat4(1.f), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)) *
      glm::translate(glm::mat4(1.f), -light_pos);
  shadow_view_mats[3] = // -y
      glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f)) *
      glm::translate(glm::mat4(1.f), -light_pos);
  shadow_view_mats[4] = // +z
      glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)) *
      glm::translate(glm::mat4(1.f), -light_pos);
  shadow_view_mats[5] = // -z
      glm::mat4(1.f) * glm::translate(glm::mat4(1.f), -light_pos);
      
  glGenFramebuffers(1, &gl_shadow_fbo);

  glGenRenderbuffers(1, &gl_shadow_rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, gl_shadow_rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kShadowTexWidth,
                        kShadowTexHeight);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glBindFramebuffer(GL_FRAMEBUFFER, gl_shadow_fbo);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, gl_shadow_rbo);
      
  shadow_proj_mat = glm::perspective(glm::radians(90.f), 1.f, kShadowNearPlane, 
                                     kShadowFarPlane);

  GLint far_plane_loc = glGetUniformLocation(gl_shadow_program, "far_plane");
  glUniform1f(far_plane_loc, kShadowFarPlane);                                   

  GLint light_pos_loc = glGetUniformLocation(gl_shadow_program, "light_pos");
  glUniform3fv(light_pos_loc, 1, glm::value_ptr(light_pos));
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

  glm::vec3 ambient_I = glm::vec3(0.8f, 0.8f, 0.8f);
  glm::vec3 diffuse_I = glm::vec3(0.3f, 0.3f, 0.3f);
  glm::vec3 specular_I = glm::vec3(1.f, 1.f, 1.f);
  float shininess = 8.f;

  camera->SetCameraPos(glm::vec3(0.f, 5.f, 12.5f));

  GLint far_plane_loc = glGetUniformLocation(gl_program, "far_plane");
  glUniform1f(far_plane_loc, kShadowFarPlane);      

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

  for (size_t i = 0; i < 6; ++i) {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                           gl_shadow_tex, 0);

    glViewport(0, 0, kShadowTexWidth, kShadowTexHeight);
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(gl_shadow_program);

    for (size_t j = 0; j < model->meshes.size(); ++j) {
      const utils::Mesh& mesh = model->meshes[j];

      glm::mat4 model_mat = glm::scale(glm::mat4(1.f), glm::vec3(5.f, 5.f, 5.f));
      glm::mat4 mvp_mat = shadow_proj_mat * shadow_view_mats[i] * model_mat;

      GLint model_mat_loc = glGetUniformLocation(gl_shadow_program, "model_mat");
      glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, glm::value_ptr(model_mat));
      
      GLint mvp_mat_loc = glGetUniformLocation(gl_shadow_program, "mvp_mat");
      glUniformMatrix4fv(mvp_mat_loc, 1, GL_FALSE, glm::value_ptr(mvp_mat));

      glBindVertexArray(gl_shadow_vao);

      glBindBuffer(GL_ARRAY_BUFFER, gl_pos_vbos[j]);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

      glDrawArrays(GL_TRIANGLES, 0, mesh.num_verts);
    }
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

    // TODO: Replace the identity mat with the view mat of the light.
    glm::mat4 shadow_view_mat = glm::translate(glm::mat4(1.f), glm::vec3(0.f, -9.f, 0.f));
    glm::mat4 shadow_mat = shadow_proj_mat * shadow_view_mat * model_mat;
    GLint shadow_mat_loc = glGetUniformLocation(gl_program, "shadow_mat");
    glUniformMatrix4fv(shadow_mat_loc, 1, GL_FALSE, glm::value_ptr(shadow_mat));

    GLint camera_pos_loc = glGetUniformLocation(gl_program, "camera_pos");
    glUniform3fv(camera_pos_loc, 1, glm::value_ptr(camera->GetCameraPos()));

    GLint ambient_color_loc = glGetUniformLocation(gl_program, "ambient_color");
    glUniform3fv(ambient_color_loc, 1, glm::value_ptr(mesh.materials[0].ambient_color));

    GLint specular_color_loc = glGetUniformLocation(gl_program, "specular_color");
    glUniform3fv(specular_color_loc, 1, glm::value_ptr(mesh.materials[0].specular_color));

    GLint shininess_loc = glGetUniformLocation(gl_program, "shininess");
    glUniform1f(shininess_loc, mesh.materials[0].shininess);

    GLint shadow_tex_loc = glGetUniformLocation(gl_program, "shadow_tex");
    glUniform1i(shadow_tex_loc, 1);

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
  glDeleteRenderbuffers(1, &gl_shadow_rbo);
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