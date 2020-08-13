#version 430 core

in vec3 frag_pos;
in vec3 frag_normal;
in vec4 frag_shadow_pos;

out vec4 out_color;

uniform vec3 camera_pos;

uniform vec3 light_pos;
uniform vec3 ambient_I;
uniform vec3 diffuse_I;
uniform vec3 specular_I;
uniform float shininess;

uniform vec3 ambient_color;
uniform vec3 specular_color;

uniform sampler2D shadow_tex;

void main() {
  vec3 light_v = normalize(light_pos - frag_pos);
  vec3 view_v = normalize(camera_pos - frag_pos);

  vec3 half_v = normalize((light_v + view_v) / 2.0);
  vec3 normal_v = normalize(frag_normal);

  vec3 ambient = ambient_I * ambient_color;
  vec3 diffuse = diffuse_I * clamp(dot(light_v, normal_v), 0.0, 1.0);
  vec3 specular = specular_I * pow(clamp(dot(half_v, normal_v), 0.0, 1.0), shininess) * 
      specular_color;

  vec3 shadow_coords = (frag_shadow_pos.xyz / frag_shadow_pos.w) * 0.5 + 0.5;
  float shadow_tex_depth = texture2D(shadow_tex, shadow_coords.xy).r;
  float shadow_occlude = shadow_coords.z - 0.005 < shadow_tex_depth ? 1.0 : 0.0;

  vec3 intensity = ambient + (diffuse + specular) * shadow_occlude;
  out_color = vec4(intensity, 1.0);
}