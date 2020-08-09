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
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> material_data;
  std::string warn_str, err_str;

  if (!tinyobj::LoadObj(&attribs, &shapes, &material_data, &warn_str,
      &err_str, path.c_str(), material_dir.c_str())) {
    return nullptr;
  }

  model->meshes.resize(shapes.size());

  size_t mesh_idx = 0;
  for (tinyobj::shape_t shape : shapes) {
    Mesh& mesh = model->meshes[mesh_idx];

    mesh.num_verts = shape.mesh.indices.size();

    mesh.positions.resize(mesh.num_verts * sizeof(glm::vec3));
    mesh.normals.resize(mesh.num_verts * sizeof(glm::vec3));
    mesh.texcoords.resize(mesh.num_verts * sizeof(glm::vec2));

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
          mesh.positions[vert_idx].x = attribs.vertices[base_idx + 0];
          mesh.positions[vert_idx].y = attribs.vertices[base_idx + 1];
          mesh.positions[vert_idx].z = attribs.vertices[base_idx + 2];
        }
        if (vert_indices.normal_index != -1) {
          size_t base_idx = vert_indices.normal_index * 3;
          mesh.normals[vert_idx].x = attribs.normals[base_idx + 0];
          mesh.normals[vert_idx].y = attribs.normals[base_idx + 1];
          mesh.normals[vert_idx].z = attribs.normals[base_idx + 2];
        }
        if (vert_indices.texcoord_index != -1) {
          size_t base_idx = vert_indices.texcoord_index * 2;
          mesh.texcoords[vert_idx].s = attribs.texcoords[base_idx + 0];
          mesh.texcoords[vert_idx].t = attribs.texcoords[base_idx + 1];
        }
        ++vert_idx;
      }
    }

    ++mesh_idx;
  }

  return std::move(model);
}

} // namespace utils