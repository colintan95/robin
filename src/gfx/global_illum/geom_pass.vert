#version 430 core

layout(location = 0) in vec3 vert_pos;
layout(location = 1) in vec3 vert_normal;
layout(location = 2) in vec2 vert_texcoord;

uniform mat4 mvp_mat;

void main() {
  gl_Position = mvp_mat * vec4(vert_pos, 1.0);
}