#version 430 core

in vec3 frag_pos;

out float out_color;

uniform vec3 light_pos;
uniform float far_plane;

void main() {
  out_color = length(frag_pos - light_pos) / far_plane;
}