#ifndef UTILS_MODEL_H_
#define UTILS_MODEL_H_

#include <glm/glm.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace utils {

struct Model {
  std::vector<glm::vec3> positions;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec2> texcoords;
  uint32_t num_verts;
};

std::shared_ptr<Model> LoadModelFromFile(const std::string& path, 
                                         const std::string& material_dir);

} // namespace utils

#endif // UTILS_MODEL_H_