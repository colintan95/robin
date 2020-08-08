#version 430 core

layout(location = 0) in vec2 vert_pos;

void main() {
  gl_Position = vec4(vert_pos, 0.0, 1.0);
}