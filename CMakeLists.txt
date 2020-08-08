cmake_minimum_required(VERSION 3.15)
project(robin)

set(CMAKE_CXX_STANDARD 17)

set(SRC_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/src")

# OpenGL
find_package(OpenGL)
if(OPENGL_FOUND)
  message(STATUS "Found OpenGL")
else()
  message(FATAL_ERROR "Unable to find OpenGL")
endif()

# GLM
add_library(glm INTERFACE IMPORTED)
set_property(TARGET glm PROPERTY INTERFACE_INCLUDE_DIRECTORIES 
    "${CMAKE_SOURCE_DIR}/third_party/glm")

# GLFW
add_library(glfw STATIC IMPORTED)
set_property(TARGET glfw PROPERTY IMPORTED_LOCATION
    "${CMAKE_SOURCE_DIR}/third_party/glfw-3.3/lib-vc2019/glfw3.lib")
set_property(TARGET glfw PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    "${CMAKE_SOURCE_DIR}/third_party/glfw-3.3/include")

# GLEW
add_library(glew STATIC IMPORTED)
set_property(TARGET glew PROPERTY IMPORTED_LOCATION
    "${CMAKE_SOURCE_DIR}/third_party/glew-2.1.0/lib/Release/x64/glew32s.lib")
set_property(TARGET glew PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    "${CMAKE_SOURCE_DIR}/third_party/glew-2.1.0/include")

# STB
add_library(stb INTERFACE IMPORTED)
set_property(TARGET stb PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    "${CMAKE_SOURCE_DIR}/third_party/stb/include")

# TinyObjLoader
add_library(tinyobjloader INTERFACE IMPORTED)
set_property(TARGET tinyobjloader PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    "${CMAKE_SOURCE_DIR}/third_party/tinyobjloader-2.0.0/include")

add_subdirectory(src)