#version 430 core

layout(location = 0) in vec3 vert_pos;
layout(location = 1) in vec3 vert_normal;
layout(location = 2) in vec2 vert_texcoord;
layout(location = 3) in int vert_mtl_id;

out vec2 frag_texcoord;
flat out int frag_mtl_id;

uniform mat4 mvp_mat;

void main() {
  frag_texcoord = vert_texcoord;
  frag_mtl_id = vert_mtl_id;

  gl_Position = mvp_mat * vec4(vert_pos, 1.0);
}