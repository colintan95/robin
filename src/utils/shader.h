#ifndef UTILS_SHADER_H_
#define UTILS_SHADER_H_

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/gl.h>

#include <optional>
#include <string>

namespace utils {

std::optional<std::string> LoadShaderSource(const std::string& path);

bool CompileShader(GLuint shader, const std::string& shader_src);

} // namespace utils

#endif // UTILS_SHADER_H_