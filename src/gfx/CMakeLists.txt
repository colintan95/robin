
# Adds shaders as dependencies. Copies the shaders into the executable folder.
function(add_shaders TARGET SHADER_SRC_FILES)
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
  add_dependencies("${TARGET}" shaders)
endfunction()

add_subdirectory(hello_triangle) 