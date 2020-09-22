#version 430 core

layout(location = 0) in vec3 vert_pos;

out vec3 frag_pos;

uniform mat4 mvp_mat;
uniform mat4 model_mat;

void main() {
  frag_pos = (model_mat * vec4(vert_pos, 1.0)).xyz;
  gl_Position = mvp_mat * vec4(vert_pos, 1.0);
}