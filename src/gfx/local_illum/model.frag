#version 430 core

in vec2 frag_texcoord;
out vec4 out_color;

uniform sampler2D tex_sampler;

void main() {
  out_color = texture(tex_sampler, frag_texcoord);
  // out_color = vec4(frag_texcoord, 0.0, 1.0);
}