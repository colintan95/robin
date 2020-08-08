add_executable(hello_triangle "main.cpp")

target_include_directories(hello_triangle PRIVATE ${SRC_INCLUDE_DIR})

target_link_libraries(hello_triangle PRIVATE glfw)