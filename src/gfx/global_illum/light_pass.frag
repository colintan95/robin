#version 430 core

in vec2 frag_texcoord;

out vec4 out_color;

uniform sampler2D ambient_tex;

void main() {
  out_color = texture(ambient_tex, frag_texcoord);
}