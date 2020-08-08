#ifndef UTILS_PROGRAM_H_
#define UTILS_PROGRAM_H_

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/gl.h>

namespace utils {

bool CheckProgramLinkStatus(GLuint program);

} // namespace utils

#endif // UTILS_PROGRAM_H_