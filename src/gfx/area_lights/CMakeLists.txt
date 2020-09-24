add_executable(area_lights "main.cpp")

target_link_libraries(area_lights PRIVATE glew)
target_link_libraries(area_lights PRIVATE glfw)
target_link_libraries(area_lights PRIVATE glm)
target_link_libraries(area_lights PRIVATE OpenGL::GL)

target_link_libraries(area_lights PRIVATE utils)

# Makes the src folder an include directory so that we can include any header file by specifying
# its full path from the src/ folder.
#
# E.g. the header file src/foo/bar/my.h can be included using the line:
#
#   #include "foo/bar/my.h"
#
target_include_directories(area_lights PRIVATE ${SRC_INCLUDE_DIR})

set(SHADER_SRC_FILES 
    "local_illum.frag"
    "local_illum.vert"
    "shadow_pass.frag"
    "shadow_pass.vert")
add_shaders(area_lights "${SHADER_SRC_FILES}")

# In the executable folder, creates a symlink to the assets folder.
add_custom_command(TARGET area_lights POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E create_symlink "${CMAKE_SOURCE_DIR}/assets" 
    "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/assets")