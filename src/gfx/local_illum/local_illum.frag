#version 430 core

in vec3 frag_pos;
in vec3 frag_normal;
out vec4 out_color;

uniform vec3 camera_pos;

uniform vec3 light_pos;
uniform vec3 ambient_I;
uniform vec3 diffuse_I;
uniform vec3 specular_I;
uniform float shininess;

uniform vec3 ambient_color;
uniform vec3 specular_color;

void main() {
  vec3 light_v = normalize(light_pos - frag_pos);
  vec3 view_v = normalize(camera_pos - frag_pos);

  vec3 half_v = normalize((light_v + view_v) / 2.0);
  vec3 normal_v = normalize(frag_normal);

  vec3 ambient = ambient_I * ambient_color;
  vec3 diffuse = diffuse_I * clamp(dot(light_v, normal_v), 0.0, 1.0);
  vec3 specular = specular_I * pow(clamp(dot(half_v, normal_v), 0.0, 1.0), shininess) * 
      specular_color;

  vec3 intensity = ambient + diffuse + specular;
  out_color = vec4(intensity, 1.0);
}