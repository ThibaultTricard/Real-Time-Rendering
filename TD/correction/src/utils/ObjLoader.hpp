#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <cstdint>
#include <cmath>
#include <unordered_map>

#include "Scene.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>


// Load OBJ model using tinyobjloader with separate index buffers
inline bool loadModel(const std::string& filepath,
                      const std::string& mtlSearchPath,
                      std::vector<float>& positions,
                      std::vector<uint32_t>& positionIndices) {

    tinyobj::ObjReaderConfig readerConfig;
    readerConfig.mtl_search_path = mtlSearchPath;

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filepath, readerConfig)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader error: " << reader.Error() << std::endl;
        }
        return false;
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader warning: " << reader.Warning() << std::endl;
    }

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();

    // Copy raw attribute data directly from tinyobj
    positions = attrib.vertices;


    // Extract indices for each attribute type
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            positionIndices.push_back(static_cast<uint32_t>(index.vertex_index));
        }
    }

    std::cout << "Loaded model: " << positions.size() / 3 << " positions, "
              << positionIndices.size() / 3 << " triangles" << std::endl;

    return true;
}

// Load OBJ model using tinyobjloader with separate index buffers
inline bool loadModel(const std::string& filepath,
                      const std::string& mtlSearchPath,
                      std::vector<float>& positions,
                      std::vector<float>& normals,
                      std::vector<uint32_t>& positionIndices,
                      std::vector<uint32_t>& normalIndices) {

    tinyobj::ObjReaderConfig readerConfig;
    readerConfig.mtl_search_path = mtlSearchPath;

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filepath, readerConfig)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader error: " << reader.Error() << std::endl;
        }
        return false;
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader warning: " << reader.Warning() << std::endl;
    }

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();

    // Copy raw attribute data directly from tinyobj
    positions = attrib.vertices;
    normals = attrib.normals;

    // If no normals exist, add a default normal
    if (normals.empty()) {
        normals = {0.0f, 1.0f, 0.0f};
    }


    // Extract indices for each attribute type
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            positionIndices.push_back(static_cast<uint32_t>(index.vertex_index));
            normalIndices.push_back(index.normal_index >= 0 ? static_cast<uint32_t>(index.normal_index) : 0);
        }
    }

    std::cout << "Loaded model: " << positions.size() / 3 << " positions, "
              << normals.size() / 3 << " normals, "
              << positionIndices.size() / 3 << " triangles" << std::endl;

    return true;
}


// Load OBJ model using tinyobjloader with separate index buffers
inline bool loadModel(const std::string& filepath,
                      const std::string& mtlSearchPath,
                      std::vector<float>& positions,
                      std::vector<float>& normals,
                      std::vector<float>& texcoords,
                      std::vector<uint32_t>& positionIndices,
                      std::vector<uint32_t>& normalIndices,
                      std::vector<uint32_t>& texcoordIndices) {

    tinyobj::ObjReaderConfig readerConfig;
    readerConfig.mtl_search_path = mtlSearchPath;

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filepath, readerConfig)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader error: " << reader.Error() << std::endl;
        }
        return false;
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader warning: " << reader.Warning() << std::endl;
    }

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();

    // Copy raw attribute data directly from tinyobj
    positions = attrib.vertices;
    normals = attrib.normals;
    texcoords = attrib.texcoords;

    // If no normals exist, add a default normal
    if (normals.empty()) {
        normals = {0.0f, 1.0f, 0.0f};
    }

    // If no texcoords exist, add a default texcoord
    if (texcoords.empty()) {
        texcoords = {0.0f, 0.0f};
    }

    // Extract indices for each attribute type
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            positionIndices.push_back(static_cast<uint32_t>(index.vertex_index));
            normalIndices.push_back(index.normal_index >= 0 ? static_cast<uint32_t>(index.normal_index) : 0);
            texcoordIndices.push_back(index.texcoord_index >= 0 ? static_cast<uint32_t>(index.texcoord_index) : 0);
        }
    }

    std::cout << "Loaded model: " << positions.size() / 3 << " positions, "
              << normals.size() / 3 << " normals, "
              << texcoords.size() / 2 << " texcoords, "
              << positionIndices.size() / 3 << " triangles" << std::endl;

    return true;
}

// Load OBJ model with tangent computation for normal mapping
// Tangents share the same indices as normals
inline bool loadModel(const std::string& filepath,
                      const std::string& mtlSearchPath,
                      std::vector<float>& positions,
                      std::vector<float>& normals,
                      std::vector<float>& texcoords,
                      std::vector<float>& tangents,
                      std::vector<uint32_t>& positionIndices,
                      std::vector<uint32_t>& normalIndices,
                      std::vector<uint32_t>& texcoordIndices) {

    tinyobj::ObjReaderConfig readerConfig;
    readerConfig.mtl_search_path = mtlSearchPath;

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filepath, readerConfig)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader error: " << reader.Error() << std::endl;
        }
        return false;
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader warning: " << reader.Warning() << std::endl;
    }

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();

    // Copy raw attribute data directly from tinyobj
    positions = attrib.vertices;
    normals = attrib.normals;
    texcoords = attrib.texcoords;

    // If no normals exist, add a default normal
    if (normals.empty()) {
        normals = {0.0f, 1.0f, 0.0f};
    }

    // If no texcoords exist, add a default texcoord
    if (texcoords.empty()) {
        texcoords = {0.0f, 0.0f};
    }

    // Initialize tangents (one per normal)
    size_t numNormals = normals.size() / 3;
    tangents.resize(numNormals * 3, 0.0f);

    // Compute tangents per triangle and store at normal indices
    for (const auto& shape : shapes) {
        size_t indexOffset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            size_t fv = shape.mesh.num_face_vertices[f];

            if (fv == 3) {
                tinyobj::index_t idx0 = shape.mesh.indices[indexOffset + 0];
                tinyobj::index_t idx1 = shape.mesh.indices[indexOffset + 1];
                tinyobj::index_t idx2 = shape.mesh.indices[indexOffset + 2];

                // Get positions
                float p0x = attrib.vertices[3 * idx0.vertex_index + 0];
                float p0y = attrib.vertices[3 * idx0.vertex_index + 1];
                float p0z = attrib.vertices[3 * idx0.vertex_index + 2];

                float p1x = attrib.vertices[3 * idx1.vertex_index + 0];
                float p1y = attrib.vertices[3 * idx1.vertex_index + 1];
                float p1z = attrib.vertices[3 * idx1.vertex_index + 2];

                float p2x = attrib.vertices[3 * idx2.vertex_index + 0];
                float p2y = attrib.vertices[3 * idx2.vertex_index + 1];
                float p2z = attrib.vertices[3 * idx2.vertex_index + 2];

                // Get texture coordinates
                float uv0x = 0.0f, uv0y = 0.0f;
                float uv1x = 0.0f, uv1y = 0.0f;
                float uv2x = 0.0f, uv2y = 0.0f;

                if (idx0.texcoord_index >= 0) {
                    uv0x = attrib.texcoords[2 * idx0.texcoord_index + 0];
                    uv0y = attrib.texcoords[2 * idx0.texcoord_index + 1];
                }
                if (idx1.texcoord_index >= 0) {
                    uv1x = attrib.texcoords[2 * idx1.texcoord_index + 0];
                    uv1y = attrib.texcoords[2 * idx1.texcoord_index + 1];
                }
                if (idx2.texcoord_index >= 0) {
                    uv2x = attrib.texcoords[2 * idx2.texcoord_index + 0];
                    uv2y = attrib.texcoords[2 * idx2.texcoord_index + 1];
                }

                // Compute edge vectors
                float edge1x = p1x - p0x, edge1y = p1y - p0y, edge1z = p1z - p0z;
                float edge2x = p2x - p0x, edge2y = p2y - p0y, edge2z = p2z - p0z;

                // Compute UV deltas
                float deltaUV1x = uv1x - uv0x, deltaUV1y = uv1y - uv0y;
                float deltaUV2x = uv2x - uv0x, deltaUV2y = uv2y - uv0y;

                // Compute tangent
                float denom = deltaUV1x * deltaUV2y - deltaUV2x * deltaUV1y;
                float invDenom = (std::abs(denom) > 1e-6f) ? 1.0f / denom : 0.0f;

                float tx = invDenom * (deltaUV2y * edge1x - deltaUV1y * edge2x);
                float ty = invDenom * (deltaUV2y * edge1y - deltaUV1y * edge2y);
                float tz = invDenom * (deltaUV2y * edge1z - deltaUV1y * edge2z);

                // Normalize tangent
                float len = std::sqrt(tx * tx + ty * ty + tz * tz);
                if (len > 1e-6f) {
                    tx /= len; ty /= len; tz /= len;
                } else {
                    tx = 1.0f; ty = 0.0f; tz = 0.0f;
                }

                // Store tangent at each vertex's normal index
                for (size_t v = 0; v < 3; v++) {
                    tinyobj::index_t idx = shape.mesh.indices[indexOffset + v];
                    if (idx.normal_index >= 0) {
                        size_t ni = static_cast<size_t>(idx.normal_index);
                        tangents[3 * ni + 0] = tx;
                        tangents[3 * ni + 1] = ty;
                        tangents[3 * ni + 2] = tz;
                    }
                }
            }

            indexOffset += fv;
        }
    }

    // Extract indices
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            positionIndices.push_back(static_cast<uint32_t>(index.vertex_index));
            normalIndices.push_back(index.normal_index >= 0 ? static_cast<uint32_t>(index.normal_index) : 0);
            texcoordIndices.push_back(index.texcoord_index >= 0 ? static_cast<uint32_t>(index.texcoord_index) : 0);
        }
    }

    std::cout << "Loaded model with tangents: " << positions.size() / 3 << " positions, "
              << normals.size() / 3 << " normals, "
              << tangents.size() / 3 << " tangents, "
              << texcoords.size() / 2 << " texcoords, "
              << positionIndices.size() / 3 << " triangles" << std::endl;

    return true;
}

// Helper function to compute tangents for a mesh
inline void computeMeshTangents(Mesh& mesh, const tinyobj::attrib_t& attrib,
                                 const tinyobj::shape_t& shape) {
    size_t numNormals = mesh.normals.size() / 3;
    mesh.tangents.resize(numNormals * 3, 0.0f);

    size_t indexOffset = 0;
    for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
        size_t fv = shape.mesh.num_face_vertices[f];

        if (fv == 3) {
            tinyobj::index_t idx0 = shape.mesh.indices[indexOffset + 0];
            tinyobj::index_t idx1 = shape.mesh.indices[indexOffset + 1];
            tinyobj::index_t idx2 = shape.mesh.indices[indexOffset + 2];

            float p0x = attrib.vertices[3 * idx0.vertex_index + 0];
            float p0y = attrib.vertices[3 * idx0.vertex_index + 1];
            float p0z = attrib.vertices[3 * idx0.vertex_index + 2];

            float p1x = attrib.vertices[3 * idx1.vertex_index + 0];
            float p1y = attrib.vertices[3 * idx1.vertex_index + 1];
            float p1z = attrib.vertices[3 * idx1.vertex_index + 2];

            float p2x = attrib.vertices[3 * idx2.vertex_index + 0];
            float p2y = attrib.vertices[3 * idx2.vertex_index + 1];
            float p2z = attrib.vertices[3 * idx2.vertex_index + 2];

            float uv0x = 0.0f, uv0y = 0.0f;
            float uv1x = 0.0f, uv1y = 0.0f;
            float uv2x = 0.0f, uv2y = 0.0f;

            if (idx0.texcoord_index >= 0) {
                uv0x = attrib.texcoords[2 * idx0.texcoord_index + 0];
                uv0y = attrib.texcoords[2 * idx0.texcoord_index + 1];
            }
            if (idx1.texcoord_index >= 0) {
                uv1x = attrib.texcoords[2 * idx1.texcoord_index + 0];
                uv1y = attrib.texcoords[2 * idx1.texcoord_index + 1];
            }
            if (idx2.texcoord_index >= 0) {
                uv2x = attrib.texcoords[2 * idx2.texcoord_index + 0];
                uv2y = attrib.texcoords[2 * idx2.texcoord_index + 1];
            }

            float edge1x = p1x - p0x, edge1y = p1y - p0y, edge1z = p1z - p0z;
            float edge2x = p2x - p0x, edge2y = p2y - p0y, edge2z = p2z - p0z;

            float deltaUV1x = uv1x - uv0x, deltaUV1y = uv1y - uv0y;
            float deltaUV2x = uv2x - uv0x, deltaUV2y = uv2y - uv0y;

            float denom = deltaUV1x * deltaUV2y - deltaUV2x * deltaUV1y;
            float invDenom = (std::abs(denom) > 1e-6f) ? 1.0f / denom : 0.0f;

            float tx = invDenom * (deltaUV2y * edge1x - deltaUV1y * edge2x);
            float ty = invDenom * (deltaUV2y * edge1y - deltaUV1y * edge2y);
            float tz = invDenom * (deltaUV2y * edge1z - deltaUV1y * edge2z);

            float len = std::sqrt(tx * tx + ty * ty + tz * tz);
            if (len > 1e-6f) {
                tx /= len; ty /= len; tz /= len;
            } else {
                tx = 1.0f; ty = 0.0f; tz = 0.0f;
            }

            for (size_t v = 0; v < 3; v++) {
                tinyobj::index_t idx = shape.mesh.indices[indexOffset + v];
                if (idx.normal_index >= 0) {
                    size_t ni = static_cast<size_t>(idx.normal_index);
                    mesh.tangents[3 * ni + 0] = tx;
                    mesh.tangents[3 * ni + 1] = ty;
                    mesh.tangents[3 * ni + 2] = tz;
                }
            }
        }

        indexOffset += fv;
    }
}

// Load OBJ file as a Scene with meshes and materials
inline bool loadScene(const std::string& filepath,
                      const std::string& mtlSearchPath,
                      Scene& scene) {

    tinyobj::ObjReaderConfig readerConfig;
    readerConfig.mtl_search_path = mtlSearchPath;

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filepath, readerConfig)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader error: " << reader.Error() << std::endl;
        }
        return false;
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader warning: " << reader.Warning() << std::endl;
    }

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();
    const auto& objMaterials = reader.GetMaterials();

    scene.clear();

    // Load materials
    for (const auto& objMat : objMaterials) {
        Material mat;
        mat.name = objMat.name;

        mat.ambient[0] = objMat.ambient[0];
        mat.ambient[1] = objMat.ambient[1];
        mat.ambient[2] = objMat.ambient[2];

        mat.diffuse[0] = objMat.diffuse[0];
        mat.diffuse[1] = objMat.diffuse[1];
        mat.diffuse[2] = objMat.diffuse[2];

        mat.specular[0] = objMat.specular[0];
        mat.specular[1] = objMat.specular[1];
        mat.specular[2] = objMat.specular[2];

        mat.emission[0] = objMat.emission[0];
        mat.emission[1] = objMat.emission[1];
        mat.emission[2] = objMat.emission[2];

        mat.shininess = objMat.shininess;
        mat.opacity = objMat.dissolve;

        mat.diffuseTexture = objMat.diffuse_texname;
        mat.normalTexture = objMat.bump_texname;
        mat.specularTexture = objMat.specular_texname;

        scene.addMaterial(mat);
    }

    // Load meshes (one per shape)
    for (const auto& shape : shapes) {
        Mesh mesh;
        mesh.name = shape.name;

        // Copy attribute data
        mesh.positions = attrib.vertices;
        mesh.normals = attrib.normals;
        mesh.texcoords = attrib.texcoords;

        if (mesh.normals.empty()) {
            mesh.normals = {0.0f, 1.0f, 0.0f};
        }
        if (mesh.texcoords.empty()) {
            mesh.texcoords = {0.0f, 0.0f};
        }

        // Extract indices
        for (const auto& index : shape.mesh.indices) {
            mesh.positionIndices.push_back(static_cast<uint32_t>(index.vertex_index));
            mesh.normalIndices.push_back(index.normal_index >= 0 ? static_cast<uint32_t>(index.normal_index) : 0);
            mesh.texcoordIndices.push_back(index.texcoord_index >= 0 ? static_cast<uint32_t>(index.texcoord_index) : 0);
        }

        // Get material index (use first face's material)
        if (!shape.mesh.material_ids.empty() && shape.mesh.material_ids[0] >= 0) {
            mesh.materialIndex = shape.mesh.material_ids[0];
        }

        // Compute tangents
        computeMeshTangents(mesh, attrib, shape);

        scene.addMesh(mesh);
    }

    std::cout << "Loaded scene: " << scene.getMeshCount() << " meshes, "
              << scene.getMaterialCount() << " materials" << std::endl;

    return true;
}
