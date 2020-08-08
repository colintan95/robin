#include "utils/program.h"

#include <iostream>
#include <vector>

namespace utils {

bool CheckProgramLinkStatus(GLuint program) {
  GLint result = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &result);
  
  if (result == GL_FALSE) {
    int log_len;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);
    
    if (log_len > 0) {
      std::vector<GLchar> error_log(log_len);
      glGetProgramInfoLog(program, log_len, nullptr, &error_log[0]);
      std::cerr << &error_log[0] << std::endl;
    }
    return false;
  }
  return true;
}

} // namespace utils