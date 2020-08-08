#include "utils/shader.h"

#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace utils {

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

} // namespace utils