add_executable(hello_triangle "main.cpp")

target_link_libraries(hello_triangle PRIVATE glew)
target_link_libraries(hello_triangle PRIVATE glfw)
target_link_libraries(hello_triangle PRIVATE glm)
target_link_libraries(hello_triangle PRIVATE OpenGL::GL)

target_link_libraries(hello_triangle PRIVATE utils)

#======================================================================

# Makes the src folder an include directory so that we can include any header file by specifying
# its full path from the src/ folder
#
# E.g. the header file src/foo/bar/my.h can be included using the line:
#
#   #include "foo/bar/my.h"
#
target_include_directories(hello_triangle PRIVATE ${SRC_INCLUDE_DIR})

#======================================================================

# Adds shaders as dependencies. Copies the shaders into the executable folder.

set(SHADER_SRC_FILES 
    "triangle.frag"
    "triangle.vert")

# TODO: Make this a function
foreach(file ${SHADER_SRC_FILES})
  set(src "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
  set(dst "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${file}")
  add_custom_command(
      OUTPUT "${dst}"
      COMMAND ${CMAKE_COMMAND} -E copy "${src}" "${dst}"
      MAIN_DEPENDENCY "${src}"
      COMMENT "Copying ${file} to executable folder: ${dst}")
endforeach(file)

add_custom_target(shaders ALL DEPENDS ${SHADER_SRC_FILES})
add_dependencies(hello_triangle shaders)

#======================================================================