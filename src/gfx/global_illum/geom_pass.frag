#version 430 core

in vec2 frag_texcoord;
flat in int frag_mtl_id;

out vec4 out_color;

struct Material {
  vec3 Ka; // ambient color
  vec3 Kd; // diffuse color
  vec3 Ks; // specular color
  float Ns; // specular exponent (i.e. shininess)

  sampler2D tex_a; // ambient texture
};

uniform Material mtls[5];

void main() {
  out_color = vec4(mtls[frag_mtl_id].Ka, 1.0) * texture(mtls[frag_mtl_id].tex_a, frag_texcoord);
}