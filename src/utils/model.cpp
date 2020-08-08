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

  assert(shape_data.size() == 1);
  tinyobj::shape_t shape = shape_data[0];

  model->num_verts = shape.mesh.indices.size();

  model->positions.resize(model->num_verts * sizeof(glm::vec3));
  model->normals.resize(model->num_verts * sizeof(glm::vec3));
  model->texcoords.resize(model->num_verts * sizeof(glm::vec2));

  // Code adapted from vulkan-tutorial.com
  // Triangle faces only - for now.
  size_t vert_idx = 0;
  size_t indices_idx = 0;
  for (unsigned int num_verts : shape.mesh.num_face_vertices) {
    if (num_verts != 3) {
      std::cerr << "Face is not a triangle. Failing." << std::endl;
      return nullptr;
    }

    for (size_t i = 0; i < num_verts; ++i) {
      auto vert_indices = shape.mesh.indices[vert_idx];
      
      if (vert_indices.vertex_index != -1) {
        size_t base_idx = vert_indices.vertex_index * 3;
        model->positions[vert_idx].x = attribs.vertices[base_idx + 0];
        model->positions[vert_idx].y = attribs.vertices[base_idx + 1];
        model->positions[vert_idx].z = attribs.vertices[base_idx + 2];
      }
      if (vert_indices.normal_index != -1) {
        size_t base_idx = vert_indices.normal_index * 3;
        model->normals[vert_idx].x = attribs.normals[base_idx + 0];
        model->normals[vert_idx].y = attribs.normals[base_idx + 1];
        model->normals[vert_idx].z = attribs.normals[base_idx + 2];
      }
      if (vert_indices.texcoord_index != -1) {
        size_t base_idx = vert_indices.texcoord_index * 2;
        model->texcoords[vert_idx].s = attribs.texcoords[base_idx + 0];
        model->texcoords[vert_idx].t = attribs.texcoords[base_idx + 1];
      }
      ++vert_idx;
    }
  }

  return std::move(model);
}

} // namespace utils