#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/gl.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>
#include <memory>
#include "utils/model.h"
#include "utils/shader.h"
#include "utils/program.h"

constexpr int kWindowWidth = 1920;
constexpr int kWindowHeight = 1080;
const char* kWindowTitle = "Hello Cube";

const glm::vec3 kVertices[] = {{-0.5f, -0.5f, 0.f}, {0.5f, -0.5f, 0.f}, {0.f, 0.5f, 0.f}};

GLuint gl_program;
GLuint gl_vao;
GLuint gl_vbo;
std::shared_ptr<utils::Model> model;

void Initialize() {
  glClearColor(0.f, 0.f, 0.f, 1.f);

  gl_program = glCreateProgram();
  if (!gl_program) {
    std::cerr << "Could not create gl_program." << std::endl;
    exit(1);
  }

  GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
  if (auto src_opt = utils::LoadShaderSource("triangle.vert")) {
    if (!utils::CompileShader(vert_shader, src_opt.value())) {
      std::cerr << "Could not compile vertex shader." << std::endl;
      exit(1);
    }
  } else {
    std:: cerr << "Could not load shader from file triangle.vert." << std::endl;
    exit(1);
  }

  GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
  if (auto src_opt = utils::LoadShaderSource("triangle.frag")) {
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

  model = utils::LoadModelFromFile("assets/cube/cube.obj", "assets/cube");
  if (model == nullptr) {
    std::cerr << "Could not load model." << std::endl;
    exit(1);
  }

  glGenBuffers(1, &gl_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, gl_vbo);
  glBufferData(GL_ARRAY_BUFFER, model->positions.size() * 3, glm::value_ptr(model->positions[0]), 
               GL_STATIC_DRAW);
}

void RenderPass() {
  glViewport(0, 0, kWindowWidth, kWindowHeight);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(gl_program);
  glBindVertexArray(gl_vao);

  glBindBuffer(GL_ARRAY_BUFFER, gl_vbo);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

  glDrawArrays(GL_TRIANGLES, 0, model->num_verts);
}

void Cleanup() {
  glDeleteBuffers(1, &gl_vbo);
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