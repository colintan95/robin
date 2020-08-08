add_executable(local_illum "main.cpp")

target_link_libraries(local_illum PRIVATE glew)
target_link_libraries(local_illum PRIVATE glfw)
target_link_libraries(local_illum PRIVATE glm)
target_link_libraries(local_illum PRIVATE OpenGL::GL)

target_link_libraries(local_illum PRIVATE utils)

# Makes the src folder an include directory so that we can include any header file by specifying
# its full path from the src/ folder
#
# E.g. the header file src/foo/bar/my.h can be included using the line:
#
#   #include "foo/bar/my.h"
#
target_include_directories(local_illum PRIVATE ${SRC_INCLUDE_DIR})

set(SHADER_SRC_FILES 
    "model.frag"
    "model.vert")
add_shaders(local_illum "${SHADER_SRC_FILES}")

# In the executable folder, creates a symlink to the assets folder.
add_custom_command(TARGET local_illum POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E create_symlink "${CMAKE_SOURCE_DIR}/assets" 
    "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/assets")