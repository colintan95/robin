#version 430 core

layout(location = 0) in vec3 vert_pos;
layout(location = 1) in vec3 vert_normal;

out vec3 frag_pos;
out vec3 frag_normal;
out vec4 frag_shadow_pos;

uniform mat4 model_mat;
uniform mat4 mvp_mat;
uniform mat3 normal_mat;
uniform mat4 shadow_mat;

void main() {
  frag_pos = (model_mat * vec4(vert_pos, 1.0)).xyz;
  frag_normal = mat3(model_mat) * vert_normal;
  frag_shadow_pos = shadow_mat * vec4(vert_pos, 1.0);
  
  gl_Position = mvp_mat * vec4(vert_pos, 1.0);
}