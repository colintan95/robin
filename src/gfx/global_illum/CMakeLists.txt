add_executable(global_illum "main.cpp")

target_link_libraries(global_illum PRIVATE glew)
target_link_libraries(global_illum PRIVATE glfw)
target_link_libraries(global_illum PRIVATE glm)
target_link_libraries(global_illum PRIVATE OpenGL::GL)

target_link_libraries(global_illum PRIVATE utils)

# Makes the src folder an include directory so that we can include any header file by specifying
# its full path from the src/ folder.
#
# E.g. the header file src/foo/bar/my.h can be included using the line:
#
#   #include "foo/bar/my.h"
#
target_include_directories(global_illum PRIVATE ${SRC_INCLUDE_DIR})

set(SHADER_SRC_FILES 
    "geom_pass.frag"
    "geom_pass.vert"
    "light_pass.frag"
    "light_pass.vert")
add_shaders(global_illum "${SHADER_SRC_FILES}")

# In the executable folder, creates a symlink to the assets folder.
add_custom_command(TARGET global_illum POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E create_symlink "${CMAKE_SOURCE_DIR}/assets" 
    "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/assets")