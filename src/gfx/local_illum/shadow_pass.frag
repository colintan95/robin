#version 430 core

in vec3 frag_pos;

out vec4 out_color;

uniform vec3 light_pos;
uniform float far_plane;

void main() {
  vec3 light_v = frag_pos - light_pos;

  out_color = vec4(length(light_v) / far_plane, 0, 0, 0);
}