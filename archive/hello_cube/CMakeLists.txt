add_executable(hello_cube "main.cpp")

target_link_libraries(hello_cube PRIVATE glew)
target_link_libraries(hello_cube PRIVATE glfw)
target_link_libraries(hello_cube PRIVATE glm)
target_link_libraries(hello_cube PRIVATE OpenGL::GL)

target_link_libraries(hello_cube PRIVATE utils)

# Makes the src folder an include directory so that we can include any header file by specifying
# its full path from the src/ folder
#
# E.g. the header file src/foo/bar/my.h can be included using the line:
#
#   #include "foo/bar/my.h"
#
target_include_directories(hello_cube PRIVATE ${SRC_INCLUDE_DIR})

set(SHADER_SRC_FILES 
    "model.frag"
    "model.vert")
add_shaders(hello_cube "${SHADER_SRC_FILES}")

# In the executable folder, creates a symlink to the assets folder.
add_custom_command(TARGET hello_cube POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E create_symlink "${CMAKE_SOURCE_DIR}/assets" 
    "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/assets")