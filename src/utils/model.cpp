#include "utils/model.h"

#include <glm/glm.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"

#include <iostream>
#include <utility>

namespace utils {

std::shared_ptr<Model> LoadModelFromFile(const std::string& path, 
                                         const std::string& material_dir) {
  auto model = std::make_shared<Model>();

  tinyobj::attrib_t attribs;
  std::vector<tinyobj::shape_t> shape_data;
  std::vector<tinyobj::material_t> material_data;
  std::string warn_str, err_str;

  if (!tinyobj::LoadObj(&attribs, &shape_data, &material_data, &warn_str,
      &err_str, path.c_str(), material_dir.c_str())) {
    return nullptr;
  }

  // Code adapted from vulkan-tutorial.com
  for (const auto& shape : shape_data) {
    // Triangle faces only - for now.
    for (unsigned int num_verts : shape.mesh.num_face_vertices) {
      if (num_verts != 3) {
        std::cerr << "Face is not a triangle. Failing." << std::endl;
        return nullptr;
      }

      model->num_verts = shape.mesh.num_face_vertices.size() * 3;
      if (model->num_verts == 0) {
        std::cerr << "Mesh has no vertex data." << std::endl;
        return nullptr;
      }

      size_t indices_idx = 0;
      for (unsigned char num_verts : shape.mesh.num_face_vertices) {
        for (unsigned char i = 0; i < num_verts; ++i) {
          auto vert_indices = shape.mesh.indices[indices_idx + i];

          if (vert_indices.vertex_index != -1) {
            size_t base_idx = vert_indices.vertex_index * 3;
            model->positions.push_back({attribs.vertices[base_idx + 0], 
                                        attribs.vertices[base_idx + 1],
                                        attribs.vertices[base_idx + 2]});
          }
          if (vert_indices.normal_index != -1) {
            size_t base_idx = vert_indices.normal_index * 3;
            model->normals.push_back({attribs.normals[base_idx + 0], 
                                      attribs.normals[base_idx + 1],
                                      attribs.normals[base_idx + 2]});
          }
          if (vert_indices.texcoord_index != -1) {
            size_t base_idx = vert_indices.texcoord_index * 2;
            model->texcoords.push_back({attribs.texcoords[base_idx + 0], 
                                        attribs.texcoords[base_idx + 1]});
          }
        }

        indices_idx += num_verts;
      }
    }
  }

  return std::move(model);
}

} // namespace utils