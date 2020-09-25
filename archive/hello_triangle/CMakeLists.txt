add_executable(hello_triangle "main.cpp")

target_link_libraries(hello_triangle PRIVATE glew)
target_link_libraries(hello_triangle PRIVATE glfw)
target_link_libraries(hello_triangle PRIVATE glm)
target_link_libraries(hello_triangle PRIVATE OpenGL::GL)

target_link_libraries(hello_triangle PRIVATE utils)

# Makes the src folder an include directory so that we can include any header file by specifying
# its full path from the src/ folder
#
# E.g. the header file src/foo/bar/my.h can be included using the line:
#
#   #include "foo/bar/my.h"
#
target_include_directories(hello_triangle PRIVATE ${SRC_INCLUDE_DIR})

set(SHADER_SRC_FILES 
    "triangle.frag"
    "triangle.vert")
add_shaders(hello_triangle "${SHADER_SRC_FILES}")