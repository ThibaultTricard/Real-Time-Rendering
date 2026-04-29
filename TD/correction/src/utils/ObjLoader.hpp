#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <cstdint>
#include <cmath>
#include <unordered_map>
#include <map>
#include <tuple>

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
inline bool loadModel(const std::string& filepath,
                      const std::string& mtlSearchPath,
                      std::vector<float>& positions,
                      std::vector<float>& normals,
                      std::vector<float>& texcoords,
                      std::vector<float>& tangents,
                      std::vector<uint32_t>& positionIndices,
                      std::vector<uint32_t>& normalIndices,
                      std::vector<uint32_t>& texcoordIndices,
                      std::vector<uint32_t>& tangentIndices) {

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

    positions = attrib.vertices;
    normals   = attrib.normals;
    texcoords = attrib.texcoords;

    if (normals.empty())   normals   = {0.0f, 1.0f, 0.0f};
    if (texcoords.empty()) texcoords = {0.0f, 0.0f};

    // Build flat index arrays first
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            positionIndices.push_back(static_cast<uint32_t>(index.vertex_index));
            normalIndices  .push_back(index.normal_index   >= 0 ? static_cast<uint32_t>(index.normal_index)   : 0);
            texcoordIndices.push_back(index.texcoord_index >= 0 ? static_cast<uint32_t>(index.texcoord_index) : 0);
        }
    }

    // Deduplicate tangents by (pi, ni, ti) — full vertex identity
    std::map<std::tuple<uint32_t,uint32_t,uint32_t>, uint32_t> tangentIndexMap;
    std::vector<std::array<float,3>> tangentAccum;
    tangentIndices.resize(positionIndices.size());

    size_t numTriangles = positionIndices.size() / 3;
    for (size_t f = 0; f < numTriangles; f++) {
        uint32_t pi0 = positionIndices[3*f+0], pi1 = positionIndices[3*f+1], pi2 = positionIndices[3*f+2];
        uint32_t ni0 = normalIndices[3*f+0],   ni1 = normalIndices[3*f+1],   ni2 = normalIndices[3*f+2];
        uint32_t ti0 = texcoordIndices[3*f+0], ti1 = texcoordIndices[3*f+1], ti2 = texcoordIndices[3*f+2];

        float p0x = positions[3*pi0], p0y = positions[3*pi0+1], p0z = positions[3*pi0+2];
        float p1x = positions[3*pi1], p1y = positions[3*pi1+1], p1z = positions[3*pi1+2];
        float p2x = positions[3*pi2], p2y = positions[3*pi2+1], p2z = positions[3*pi2+2];

        float uv0x = texcoords[2*ti0], uv0y = texcoords[2*ti0+1];
        float uv1x = texcoords[2*ti1], uv1y = texcoords[2*ti1+1];
        float uv2x = texcoords[2*ti2], uv2y = texcoords[2*ti2+1];

        float e1x = p1x-p0x, e1y = p1y-p0y, e1z = p1z-p0z;
        float e2x = p2x-p0x, e2y = p2y-p0y, e2z = p2z-p0z;
        float d1x = uv1x-uv0x, d1y = uv1y-uv0y;
        float d2x = uv2x-uv0x, d2y = uv2y-uv0y;

        float det = d1x*d2y - d2x*d1y;
        float tx = 0.0f, ty = 0.0f, tz = 0.0f;
        if (std::abs(det) > 1e-6f) {
            tx = (d2y*e1x - d1y*e2x) / det;
            ty = (d2y*e1y - d1y*e2y) / det;
            tz = (d2y*e1z - d1y*e2z) / det;
        }

        uint32_t pis[3] = {pi0, pi1, pi2};
        uint32_t nis[3] = {ni0, ni1, ni2};
        uint32_t tis[3] = {ti0, ti1, ti2};
        for (int v = 0; v < 3; v++) {
            auto key = std::make_tuple(pis[v], nis[v], tis[v]);
            auto [it, inserted] = tangentIndexMap.emplace(key, (uint32_t)tangentAccum.size());
            if (inserted) tangentAccum.push_back({tx, ty, tz});
            else { tangentAccum[it->second][0] += tx; tangentAccum[it->second][1] += ty; tangentAccum[it->second][2] += tz; }
            tangentIndices[3*f+v] = it->second;
        }
    }

    tangents.reserve(tangentAccum.size() * 3);
    for (auto& t : tangentAccum) {
        float len = std::sqrt(t[0]*t[0] + t[1]*t[1] + t[2]*t[2]);
        if (len > 1e-6f) { t[0] /= len; t[1] /= len; t[2] /= len; }
        else              { t[0] = 1.0f; t[1] = 0.0f; t[2] = 0.0f; }
        tangents.push_back(t[0]); tangents.push_back(t[1]); tangents.push_back(t[2]);
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
    std::map<std::tuple<uint32_t,uint32_t,uint32_t>, uint32_t> tangentIndexMap;
    std::vector<std::array<float,3>> tangentAccum;

    size_t numTriangles = mesh.positionIndices.size() / 3;
    mesh.tangentIndices.resize(mesh.positionIndices.size());

    for (size_t f = 0; f < numTriangles; f++) {
        uint32_t pi0 = mesh.positionIndices[3*f+0], pi1 = mesh.positionIndices[3*f+1], pi2 = mesh.positionIndices[3*f+2];
        uint32_t ni0 = mesh.normalIndices[3*f+0],   ni1 = mesh.normalIndices[3*f+1],   ni2 = mesh.normalIndices[3*f+2];
        uint32_t ti0 = mesh.texcoordIndices[3*f+0], ti1 = mesh.texcoordIndices[3*f+1], ti2 = mesh.texcoordIndices[3*f+2];

        float p0x = attrib.vertices[3*pi0], p0y = attrib.vertices[3*pi0+1], p0z = attrib.vertices[3*pi0+2];
        float p1x = attrib.vertices[3*pi1], p1y = attrib.vertices[3*pi1+1], p1z = attrib.vertices[3*pi1+2];
        float p2x = attrib.vertices[3*pi2], p2y = attrib.vertices[3*pi2+1], p2z = attrib.vertices[3*pi2+2];

        float uv0x = attrib.texcoords[2*ti0], uv0y = attrib.texcoords[2*ti0+1];
        float uv1x = attrib.texcoords[2*ti1], uv1y = attrib.texcoords[2*ti1+1];
        float uv2x = attrib.texcoords[2*ti2], uv2y = attrib.texcoords[2*ti2+1];

        float e1x = p1x-p0x, e1y = p1y-p0y, e1z = p1z-p0z;
        float e2x = p2x-p0x, e2y = p2y-p0y, e2z = p2z-p0z;
        float d1x = uv1x-uv0x, d1y = uv1y-uv0y;
        float d2x = uv2x-uv0x, d2y = uv2y-uv0y;

        float det = d1x*d2y - d2x*d1y;
        float tx = 0.0f, ty = 0.0f, tz = 0.0f;
        if (std::abs(det) > 1e-6f) {
            tx = (d2y*e1x - d1y*e2x) / det;
            ty = (d2y*e1y - d1y*e2y) / det;
            tz = (d2y*e1z - d1y*e2z) / det;
        }

        uint32_t pis[3] = {pi0, pi1, pi2};
        uint32_t nis[3] = {ni0, ni1, ni2};
        uint32_t tis[3] = {ti0, ti1, ti2};
        for (int v = 0; v < 3; v++) {
            auto key = std::make_tuple(pis[v], nis[v], tis[v]);
            auto [it, inserted] = tangentIndexMap.emplace(key, (uint32_t)tangentAccum.size());
            if (inserted) tangentAccum.push_back({tx, ty, tz});
            else { tangentAccum[it->second][0] += tx; tangentAccum[it->second][1] += ty; tangentAccum[it->second][2] += tz; }
            mesh.tangentIndices[3*f+v] = it->second;
        }
    }

    mesh.tangents.reserve(tangentAccum.size() * 3);
    for (auto& t : tangentAccum) {
        float len = std::sqrt(t[0]*t[0] + t[1]*t[1] + t[2]*t[2]);
        if (len > 1e-6f) { t[0] /= len; t[1] /= len; t[2] /= len; }
        else              { t[0] = 1.0f; t[1] = 0.0f; t[2] = 0.0f; }
        mesh.tangents.push_back(t[0]); mesh.tangents.push_back(t[1]); mesh.tangents.push_back(t[2]);
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
