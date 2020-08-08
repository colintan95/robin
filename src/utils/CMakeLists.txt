target_sources(utils
  PUBLIC
    "image.h"
    "model.h"
    "program.h"
    "shader.h"
  PRIVATE
    "image.cpp"
    "model.cpp"
    "program.cpp"
    "shader.cpp")

target_link_libraries(utils PRIVATE glew)
target_link_libraries(utils PRIVATE glm)
target_link_libraries(utils PRIVATE OpenGL::GL)
target_link_libraries(utils PRIVATE stb)
target_link_libraries(utils PRIVATE tinyobjloader)

#======================================================================

# Makes the src folder an include directory so that we can include any header file by specifying
# its full path from the src/ folder
#
# E.g. the header file src/foo/bar/my.h can be included using the line:
#
#   #include "foo/bar/my.h"
#
target_include_directories(utils PRIVATE ${SRC_INCLUDE_DIR})

#======================================================================