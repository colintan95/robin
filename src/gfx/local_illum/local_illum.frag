#version 430 core

in vec3 frag_pos;
in vec3 frag_normal;
in vec2 frag_texcoord;
out vec4 out_color;

uniform vec3 ambient_I;
uniform vec3 diffuse_I;
uniform vec3 specular_I;
uniform vec3 light_pos;
uniform sampler2D tex_sampler;

void main() {
  vec3 light_vec = light_pos - frag_pos;
  vec3 diffuse = clamp(dot(light_vec, frag_normal), 0.0, 1.0) * diffuse_I;
  vec3 intensity = ambient_I + diffuse;
  out_color = vec4(intensity, 1.0) * texture(tex_sampler, frag_texcoord);
}