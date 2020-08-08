#version 430 core

layout(location = 0) in vec3 vert_pos;

uniform mat4 mvp_mat;

void main() {
  gl_Position = mvp_mat * vec4(vert_pos, 1.0);
}