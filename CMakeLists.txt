cmake_minimum_required(VERSION 3.15)
project(robin)

set(CMAKE_CXX_STANDARD 17)

set(SRC_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/src")

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

add_subdirectory(src)