#version 430 core

in vec2 frag_texcoord;
out vec4 out_color;

void main() {
  out_color = vec4(frag_texcoord, 0.0, 1.0);
}