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
#include "utils/camera.h"
#include "utils/image.h"
#include "utils/model.h"
#include "utils/shader.h"
#include "utils/program.h"

constexpr int kWindowWidth = 1920;
constexpr int kWindowHeight = 1080;
const char* kWindowTitle = "Global Illum";

constexpr float kAspectRatio = static_cast<float>(kWindowWidth) / static_cast<float>(kWindowHeight);

std::unique_ptr<utils::Camera> camera;

GLuint gl_geom_pass_program;
GLuint gl_geom_pass_vao;
GLuint gl_gbuf_fbo;
GLuint gl_gbuf_pos_tex;
GLuint gl_gbuf_normal_tex;
GLuint gl_gbuf_ambient_tex;
GLuint gl_gbuf_depth_rbo;

std::shared_ptr<utils::Model> model;
std::vector<GLuint> gl_pos_vbos;
std::vector<GLuint> gl_normal_vbos;
std::vector<GLuint> gl_texcoord_vbos;
std::vector<GLuint> gl_mtl_id_vbos;

std::unordered_map<std::string, std::shared_ptr<utils::Image>> tex_images;
std::unordered_map<std::string, GLuint> texname_to_gl_texture;
std::unordered_map<std::string, int> texname_to_tex_unit;

GLuint gl_light_pass_program;
GLuint gl_light_pass_vao;
GLuint gl_light_pass_pos_vbo;
GLuint gl_light_pass_texcoord_vbo;

glm::mat4 view_mat;
glm::mat4 proj_mat;

// Forward declarations.
void InitGeomPass();
void InitLightPass();

void Initialize() {
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.f, 0.f, 0.f, 1.f);

  InitGeomPass();
  InitLightPass();
}

void InitGeomPass() {
  gl_geom_pass_program = glCreateProgram();
  if (!gl_geom_pass_program) {
    std::cerr << "Could not create gl_geom_pass_program." << std::endl;
    exit(1);
  }

  GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
  if (auto src_opt = utils::LoadShaderSource("geom_pass.vert")) {
    if (!utils::CompileShader(vert_shader, src_opt.value())) {
      std::cerr << "Could not compile vertex shader." << std::endl;
      exit(1);
    }
  } else {
    std:: cerr << "Could not load shader from file." << std::endl;
    exit(1);
  }

  GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
  if (auto src_opt = utils::LoadShaderSource("geom_pass.frag")) {
    if (!utils::CompileShader(frag_shader, src_opt.value())) {
      std::cerr << "Could not compile fragment shader." << std::endl;
      exit(1);
    }
  } else {
    std:: cerr << "Could not load shader from file." << std::endl;
    exit(1);
  }

  glAttachShader(gl_geom_pass_program, vert_shader);
  glAttachShader(gl_geom_pass_program, frag_shader);

  glLinkProgram(gl_geom_pass_program);
  if (!utils::CheckProgramLinkStatus(gl_geom_pass_program)) {
    exit(1);
  }
  
  glDeleteShader(frag_shader);
  glDeleteShader(vert_shader);

  glGenVertexArrays(1, &gl_geom_pass_vao);

  glGenFramebuffers(1, &gl_gbuf_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, gl_gbuf_fbo);

  glGenTextures(1, &gl_gbuf_pos_tex);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, gl_gbuf_pos_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, kWindowWidth, kWindowHeight, 0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gl_gbuf_pos_tex, 0);

  glGenTextures(1, &gl_gbuf_normal_tex);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, gl_gbuf_normal_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, kWindowWidth, kWindowHeight, 0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gl_gbuf_normal_tex, 
                         0);

  glGenTextures(1, &gl_gbuf_ambient_tex);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, gl_gbuf_ambient_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, kWindowWidth, kWindowHeight, 0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gl_gbuf_ambient_tex, 
                         0);

  GLuint gbuf_attachments[] = { 
    GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2
  };
  glDrawBuffers(3, gbuf_attachments);

  glGenRenderbuffers(1, &gl_gbuf_depth_rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, gl_gbuf_depth_rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, kWindowWidth, kWindowHeight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                            gl_gbuf_depth_rbo);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "Could not create framebuffer." << std::endl;
    exit(1);
  }

  glUseProgram(gl_geom_pass_program);
  
  proj_mat = glm::perspective(glm::radians(75.f), kAspectRatio, 0.1f, 1000.f);

  model = utils::LoadModelFromFile("assets/sponza/sponza.obj", "assets/sponza");
  if (model == nullptr) {
    std::cerr << "Could not load model." << std::endl;
    exit(1);
  }

  for (const utils::Mesh& mesh : model->meshes) {
    for (const utils::Material& mtl : mesh.materials) {
      const std::string& texname = mtl.ambient_texname;
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

  for (const auto& [texname, img] : tex_images) {
    GLuint texture;
    int tex_unit = 10 + texname_to_tex_unit.size();
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0 + tex_unit);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->width, img->height, 0, GL_RGB, GL_UNSIGNED_BYTE, 
                 img->data.data());
    texname_to_gl_texture[texname] = texture;
    texname_to_tex_unit[texname] = tex_unit;
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

void InitLightPass() {
  gl_light_pass_program = glCreateProgram();
  if (!gl_light_pass_program) {
    std::cerr << "Could not create gl_light_pass_program." << std::endl;
    exit(1);
  }

  GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
  if (auto src_opt = utils::LoadShaderSource("light_pass.vert")) {
    if (!utils::CompileShader(vert_shader, src_opt.value())) {
      std::cerr << "Could not compile vertex shader." << std::endl;
      exit(1);
    }
  } else {
    std:: cerr << "Could not load shader from file." << std::endl;
    exit(1);
  }

  GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
  if (auto src_opt = utils::LoadShaderSource("light_pass.frag")) {
    if (!utils::CompileShader(frag_shader, src_opt.value())) {
      std::cerr << "Could not compile fragment shader." << std::endl;
      exit(1);
    }
  } else {
    std:: cerr << "Could not load shader from file." << std::endl;
    exit(1);
  }

  glAttachShader(gl_light_pass_program, vert_shader);
  glAttachShader(gl_light_pass_program, frag_shader);

  glLinkProgram(gl_light_pass_program);
  if (!utils::CheckProgramLinkStatus(gl_light_pass_program)) {
    exit(1);
  }
  
  glDeleteShader(frag_shader);
  glDeleteShader(vert_shader);

  glCreateVertexArrays(1, &gl_light_pass_vao);

  glm::vec3 pos_verts[] = { {-1.f, -1.f, 0.f}, {1.f, -1.f, 0.f}, {-1.f, 1.f, 0.f},
                            {-1.f, 1.f, 0.f}, {1.f, -1.f, 0.f}, {1.f, 1.f, 0.f} };
  glm::vec2 texcoord_verts[] = { {0.f, 0.f}, {1.f, 0.f}, {0.f, 1.f}, 
                                 {0.f, 1.f}, {1.f, 0.f}, {1.f, 1.f} };
  
  glGenBuffers(1, &gl_light_pass_pos_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, gl_light_pass_pos_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(pos_verts), glm::value_ptr(pos_verts[0]), GL_STATIC_DRAW);

  glGenBuffers(1, &gl_light_pass_texcoord_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, gl_light_pass_texcoord_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(texcoord_verts), glm::value_ptr(texcoord_verts[0]), 
               GL_STATIC_DRAW);

  glUseProgram(gl_light_pass_program);

  GLint ambient_tex_loc = glGetUniformLocation(gl_light_pass_program, "ambient_tex");
  glUniform1i(ambient_tex_loc, 2);
}

void RenderPass() {
  glBindFramebuffer(GL_FRAMEBUFFER, gl_gbuf_fbo);

  glViewport(0, 0, kWindowWidth, kWindowHeight);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(gl_geom_pass_program);
  glBindVertexArray(gl_geom_pass_vao);

  for (size_t i = 0; i < model->meshes.size(); ++i) {
    const utils::Mesh& mesh = model->meshes[i];
    const utils::Material& mtl = mesh.materials[0];

    glm::mat4 model_mat = glm::mat4(1.f);
    view_mat = camera->GetViewMatrix();

    glm::mat4 mv_mat = view_mat * model_mat;
    glm::mat4 mvp_mat = proj_mat * view_mat * model_mat;

    GLint mv_mat_loc = glGetUniformLocation(gl_geom_pass_program, "mv_mat");
    glUniformMatrix4fv(mv_mat_loc, 1, GL_FALSE, glm::value_ptr(mv_mat));
      
    GLint mvp_mat_loc = glGetUniformLocation(gl_geom_pass_program, "mvp_mat");
    glUniformMatrix4fv(mvp_mat_loc, 1, GL_FALSE, glm::value_ptr(mvp_mat));

    glm::mat3 normal_mat = glm::transpose(glm::inverse(glm::mat3(mv_mat)));
    GLint normal_mat_loc = glGetUniformLocation(gl_geom_pass_program, "normal_mat");
    glUniformMatrix3fv(normal_mat_loc, 1, GL_FALSE, glm::value_ptr(normal_mat)); 

    GLuint ambient_color_loc = glGetUniformLocation(gl_geom_pass_program, "mtls[0].Ka");
    glUniform3fv(ambient_color_loc, 1, glm::value_ptr(mtl.ambient_color));

    GLuint ambient_tex_loc = glGetUniformLocation(gl_geom_pass_program, "mtls[0].tex_a");
    glUniform1i(ambient_tex_loc, texname_to_tex_unit[mtl.ambient_texname]);

    glBindBuffer(GL_ARRAY_BUFFER, gl_pos_vbos[i]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, gl_normal_vbos[i]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, gl_texcoord_vbos[i]);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, gl_mtl_id_vbos[i]);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_INT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLES, 0, mesh.num_verts);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glViewport(0, 0, kWindowWidth, kWindowHeight);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(gl_light_pass_program);
  glBindVertexArray(gl_light_pass_vao);

  glBindBuffer(GL_ARRAY_BUFFER, gl_light_pass_pos_vbo);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, gl_light_pass_texcoord_vbo);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Cleanup() {
  glDeleteBuffers(1, &gl_light_pass_texcoord_vbo);
  glDeleteBuffers(1, &gl_light_pass_pos_vbo);
  glDeleteVertexArrays(1, &gl_light_pass_vao);
  glDeleteProgram(gl_light_pass_program);

  glDeleteBuffers(gl_mtl_id_vbos.size(), &gl_mtl_id_vbos[0]);
  glDeleteBuffers(gl_texcoord_vbos.size(), &gl_texcoord_vbos[0]);
  glDeleteBuffers(gl_normal_vbos.size(), &gl_normal_vbos[0]);
  glDeleteBuffers(gl_pos_vbos.size(), &gl_pos_vbos[0]);
  
  for (const auto& [texname, texture] : texname_to_gl_texture) {
    glDeleteTextures(1, &texture);
  }

  glDeleteRenderbuffers(1, &gl_gbuf_depth_rbo);
  glDeleteTextures(1, &gl_gbuf_ambient_tex);
  glDeleteTextures(1, &gl_gbuf_normal_tex);
  glDeleteTextures(1, &gl_gbuf_pos_tex);
  glDeleteFramebuffers(1, &gl_gbuf_fbo);
  glDeleteVertexArrays(1, &gl_geom_pass_vao);
  glDeleteProgram(gl_geom_pass_program);
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

  while (!glfwWindowShouldClose(glfw_window)) {
    glfwPollEvents();
    camera->Tick();

    RenderPass();
    
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