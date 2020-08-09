#version 430 core

in vec3 frag_pos;
in vec3 frag_normal;
in vec2 frag_texcoord;
flat in int frag_mtl_id;

layout(location = 0) out vec3 out_pos;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec3 out_ambient;

struct Material {
  vec3 Ka; // ambient color
  vec3 Kd; // diffuse color
  vec3 Ks; // specular color
  float Ns; // specular exponent (i.e. shininess)

  sampler2D tex_a; // ambient texture
};

uniform Material mtls[5];

void main() {
  out_pos = frag_pos;
  out_normal = frag_normal;
  out_ambient = 
      (vec4(mtls[frag_mtl_id].Ka, 1.0) * texture(mtls[frag_mtl_id].tex_a, frag_texcoord)).rgb;
}