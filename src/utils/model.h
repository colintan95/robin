#ifndef UTILS_MODEL_H_
#define UTILS_MODEL_H_

#include <glm/glm.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace utils {

enum class IllumModel {
  kInvalid = -1,
  kColorOnly = 0,
  kAmbientOnly,
  kHighlight
};

struct Material {
  glm::vec3 ambient_color;
  glm::vec3 diffuse_color;
  glm::vec3 specular_color;
  glm::vec3 emission_color;

  float shininess;

  IllumModel illum;

  std::string ambient_texname;
  std::string diffuse_texname;
  std::string specular_texname;
};

struct Mesh {
  std::vector<glm::vec3> positions;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec2> texcoords;
  std::vector<int> material_ids;
  uint32_t num_verts;

  std::vector<Material> materials;
};

struct Model {
  std::vector<Mesh> meshes;
};

std::shared_ptr<Model> LoadModelFromFile(const std::string& path, 
                                         const std::string& material_dir);

} // namespace utils

#endif // UTILS_MODEL_H_