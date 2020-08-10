#include "utils/model.h"

#include <glm/glm.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <utility>

namespace utils {

namespace {

bool LoadVertexDataForMesh(const tinyobj::shape_t& shape, const tinyobj::attrib_t& attribs,
                           Mesh* mesh) {
  assert(mesh->positions.empty());
  assert(mesh->normals.empty());
  assert(mesh->texcoords.empty());

  // IMPT: Only correct because we assume that all faces are triangles.
  mesh->num_verts = shape.mesh.num_face_vertices.size() * 3; 

  bool use_pos_data = true;
  bool use_normal_data = true;
  bool use_texcoord_data = true;

  // We won't load in data for a certain vertex type (e.g. normal, texcoord) if there's at least
  // one -1 value in the tinyobjloader data (i.e. at least one vertex that doesn't have data for 
  // that type).
  // If this happens for normals, we will generate them ourselves using the position data.
  for (const tinyobj::index_t& vert_indices : shape.mesh.indices) {
    if (vert_indices.vertex_index == -1) {
      use_pos_data = false;
    }
    if (vert_indices.normal_index == -1) {
      use_normal_data = false;
    }
    if (vert_indices.texcoord_index == -1) {
      use_texcoord_data = false;
    }
  }

  if (!use_pos_data) {
    std::cerr << "Loaded model has missing position data." << std::endl;
    return false;
  }

  if (use_pos_data) {
    mesh->positions.resize(mesh->num_verts);
  }
  if (use_normal_data) {
    mesh->normals.resize(mesh->num_verts);
  }
  if (use_texcoord_data) {
    mesh->texcoords.resize(mesh->num_verts);
  }
  
  // Code adapted from vulkan-tutorial.com
  // Triangle faces only - for now.
  size_t vert_idx = 0;
  size_t indices_idx = 0;
  for (unsigned int num_verts : shape.mesh.num_face_vertices) {
    if (num_verts != 3) {
      std::cerr << "Face is not a triangle. Failing." << std::endl;
      return false;
    }

    for (size_t i = 0; i < num_verts; ++i) {
      auto vert_indices = shape.mesh.indices[vert_idx];
      
      if (use_pos_data) {
        assert(vert_indices.vertex_index != -1);
        size_t base_idx = vert_indices.vertex_index * 3;
        mesh->positions[vert_idx].x = attribs.vertices[base_idx + 0];
        mesh->positions[vert_idx].y = attribs.vertices[base_idx + 1];
        mesh->positions[vert_idx].z = attribs.vertices[base_idx + 2];
      }
      if (use_normal_data) {
        assert(vert_indices.normal_index != -1);
        size_t base_idx = vert_indices.normal_index * 3;
        mesh->normals[vert_idx].x = attribs.normals[base_idx + 0];
        mesh->normals[vert_idx].y = attribs.normals[base_idx + 1];
        mesh->normals[vert_idx].z = attribs.normals[base_idx + 2];
      }
      if (use_texcoord_data) {
        assert(vert_indices.texcoord_index != -1);
        size_t base_idx = vert_indices.texcoord_index * 2;
        mesh->texcoords[vert_idx].s = attribs.texcoords[base_idx + 0];
        mesh->texcoords[vert_idx].t = attribs.texcoords[base_idx + 1];
      }
      ++vert_idx;
    }
  }

  // Generates the normal data if it's missing. Assumes that the position data is *not* indexed and
  // the vertices are ordered anti-clockwise.
  if (!use_normal_data) {
    mesh->normals.resize(mesh->num_verts);

    for (size_t i = 0; i < mesh->num_verts; i += 3) {
      glm::vec3 left_vec = mesh->positions[i+1] - mesh->positions[i];
      glm::vec3 right_vec = mesh->positions[i+2] - mesh->positions[i];

      mesh->normals[i] = mesh->normals[i+1] = mesh->normals[i+1] = 
          glm::normalize(glm::cross(left_vec, right_vec));
    }
  }

  return true;
}

Material CreateMaterialFromLoaderData(const tinyobj::material_t& loader_mtl) {
  Material mtl;

  mtl.ambient_color  = glm::vec3(loader_mtl.ambient[0],
                                loader_mtl.ambient[1],
                                loader_mtl.ambient[2]);
  mtl.diffuse_color  = glm::vec3(loader_mtl.diffuse[0],
                                loader_mtl.diffuse[1],
                                loader_mtl.diffuse[2]);
  mtl.specular_color = glm::vec3(loader_mtl.specular[0],
                                loader_mtl.specular[1],
                                loader_mtl.specular[2]);
  mtl.emission_color = glm::vec3(loader_mtl.emission[0],
                                loader_mtl.emission[1],
                                loader_mtl.emission[2]);
  mtl.shininess = loader_mtl.shininess;

  switch (loader_mtl.illum) {
    case 0:
      mtl.illum = IllumModel::kColorOnly;
      break;
    case 1:
      mtl.illum = IllumModel::kAmbientOnly;
      break;
    case 2:
      mtl.illum = IllumModel::kHighlight;
      break;
    default:
      mtl.illum = IllumModel::kInvalid;
      break;
  }

  if (!loader_mtl.ambient_texname.empty()) {
    mtl.ambient_texname = loader_mtl.ambient_texname;
  }
  if (!loader_mtl.diffuse_texname.empty()) {
    mtl.diffuse_texname = loader_mtl.diffuse_texname;
  }
  if (!loader_mtl.specular_texname.empty()) {
    mtl.specular_texname = loader_mtl.specular_texname;
  }

  return mtl;
}

bool LoadMaterialDataForMesh(const tinyobj::shape_t& shape, 
                             const std::vector<tinyobj::material_t>& loader_mtls, Mesh* mesh) {
  mesh->material_ids.resize(mesh->num_verts);

  std::unordered_map<int, int> loader_id_to_mtl_id;

  size_t vert_idx = 0;
  for (size_t i = 0; i < shape.mesh.num_face_vertices.size(); ++i) {
    int mtl_id = -1;

    int loader_id = shape.mesh.material_ids[i];
    if (loader_id != -1) {
      // If the material is new, creates a new material id in the conversion table.
      if (loader_id_to_mtl_id.find(loader_id) == loader_id_to_mtl_id.end()) {
        loader_id_to_mtl_id[loader_id] = static_cast<int>(mesh->materials.size());
        mesh->materials.push_back(CreateMaterialFromLoaderData(loader_mtls[loader_id]));
      }

      mtl_id = loader_id_to_mtl_id[loader_id];
    }

    for (size_t j = 0; j < shape.mesh.num_face_vertices[i]; ++j) {
      mesh->material_ids[vert_idx] = mtl_id;
      ++vert_idx;
    }
  }
  return true;
}

} // namespace

std::shared_ptr<Model> LoadModelFromFile(const std::string& path, 
                                         const std::string& material_dir) {
  auto model = std::make_shared<Model>();

  tinyobj::attrib_t attribs;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn_str, err_str;

  if (!tinyobj::LoadObj(&attribs, &shapes, &materials, &warn_str,
      &err_str, path.c_str(), material_dir.c_str())) {
    return nullptr;
  }

  // Only supports triangles for now.
  for (const tinyobj::shape_t& shape : shapes) {
    for (size_t num_verts : shape.mesh.num_face_vertices) {
      if (num_verts != 3) return false;
    }
  }

  model->meshes.resize(shapes.size());

  size_t mesh_idx = 0;
  for (const tinyobj::shape_t& shape : shapes) {
    Mesh& mesh = model->meshes[mesh_idx];

    if (!LoadVertexDataForMesh(shape, attribs, &mesh)) {
      return false;
    }
    if (!LoadMaterialDataForMesh(shape, materials, &mesh)) {
      return false;
    }

    ++mesh_idx;
  }

  return std::move(model);
}

} // namespace utils