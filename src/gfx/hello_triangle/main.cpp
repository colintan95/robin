#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/gl.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

void WindowErrorCallback(int error, const char* desc) {
  std::cerr << "GLFW Error: " << error << ": " << desc << std::endl;
}

GLuint gl_program;
GLuint gl_vao;
GLuint gl_vbo;

const glm::vec3 kVertices[] = {{-0.5f, -0.5f, 0.f}, {0.5f, -0.5f, 0.f}, {0.f, 0.5f, 0.f}};

std::optional<std::string> LoadShaderSource(const std::string& path) {
  std::ifstream file(path, std::ios::in);
  if (!file.is_open()) {
    return std::nullopt;
  }

  std::stringstream sstrm;
  sstrm << file.rdbuf();
  std::string src = sstrm.str();

  file.close();
  return src;
}

bool CompileShader(GLuint shader, const std::string& shader_src) {
  const GLchar* sources[] = { shader_src.c_str() };
  const GLint sources_lengths[] = { static_cast<GLint>(shader_src.length()) };
  glShaderSource(shader, 1, sources, sources_lengths);
  glCompileShader(shader);

  // Checks if the shader compilation succeeded.
  GLint result = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

  if (result == GL_FALSE) {
    int log_len;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);

    if (log_len > 0) {
      std::vector<GLchar> error_log(log_len);
      glGetShaderInfoLog(shader, log_len, nullptr, &error_log[0]);
      std::cerr << &error_log[0] << std::endl;
    }
    return false;
  }
  return true;
}

void Init() {
  glClearColor(0.f, 0.f, 0.f, 1.f);

  gl_program = glCreateProgram();
  if (!gl_program) {
    std::cerr << "Could not create gl_program." << std::endl;
    exit(1);
  }

  GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
  if (auto src_opt = LoadShaderSource("triangle.vert")) {
    if (!CompileShader(vert_shader, src_opt.value())) {
      std::cerr << "Could not compile vertex shader." << std::endl;
      exit(1);
    }
  } else {
    std:: cerr << "Could not load shader from file triangle.vert." << std::endl;
    exit(1);
  }

  GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
  if (auto src_opt = LoadShaderSource("triangle.frag")) {
    if (!CompileShader(frag_shader, src_opt.value())) {
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

  // Checks that the program was linked successfully.
  GLint result = GL_FALSE;
  glGetProgramiv(gl_program, GL_LINK_STATUS, &result);
  if (result == GL_FALSE) {
    int log_len;
    glGetProgramiv(gl_program, GL_INFO_LOG_LENGTH, &log_len);

    if (log_len > 0) {
      std::vector<GLchar> error_log(log_len);
      glGetProgramInfoLog(gl_program, log_len, nullptr, &error_log[0]);
      std::cerr << &error_log[0] << std::endl;
    }
    exit(1);
  }
  
  glDeleteShader(frag_shader);
  glDeleteShader(vert_shader);

  glGenVertexArrays(1, &gl_vao);

  glGenBuffers(1, &gl_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, gl_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(kVertices), glm::value_ptr(kVertices[0]), GL_STATIC_DRAW);
}

void RenderPass() {
  glViewport(0, 0, 1920, 1080);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(gl_program);
  glBindVertexArray(gl_vao);

  glBindBuffer(GL_ARRAY_BUFFER, gl_vbo);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

  glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Cleanup() {
  glDeleteBuffers(1, &gl_vbo);
  glDeleteVertexArrays(1, &gl_vao);
  glDeleteProgram(gl_program);
}

int main() {
  if (!glfwInit()) {
    std::cerr << "Could not initialize GLFW." << std::endl;
    exit(1);
  }
  glfwSetErrorCallback(WindowErrorCallback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  
  GLFWwindow* glfw_window = glfwCreateWindow(1920, 1080, "Hello, Triangle!", nullptr, nullptr);
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

  Init();

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