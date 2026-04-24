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

// Overload 1 : positions + indices seulement (normales calculees a la main)
inline bool loadModel(const std::string& filepath,
                      const std::string& mtlSearchPath,
                      std::vector<float>& positions,
                      std::vector<uint32_t>& positionIndices) {

    tinyobj::ObjReaderConfig readerConfig;
    readerConfig.mtl_search_path = mtlSearchPath;
    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filepath, readerConfig)) {
        if (!reader.Error().empty())
            std::cerr << "TinyObjReader error: " << reader.Error() << std::endl;
        return false;
    }
    if (!reader.Warning().empty())
        std::cout << "TinyObjReader warning: " << reader.Warning() << std::endl;

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();

    positions = attrib.vertices;

    for (const auto& shape : shapes)
        for (const auto& index : shape.mesh.indices)
            positionIndices.push_back(static_cast<uint32_t>(index.vertex_index));

    std::cout << "Loaded model: " << positions.size() / 3 << " positions, "
              << positionIndices.size() / 3 << " triangles" << std::endl;
    return true;
}

// Overload 2 : positions + normales OBJ + indices separes
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
        if (!reader.Error().empty())
            std::cerr << "TinyObjReader error: " << reader.Error() << std::endl;
        return false;
    }
    if (!reader.Warning().empty())
        std::cout << "TinyObjReader warning: " << reader.Warning() << std::endl;

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();

    positions = attrib.vertices;
    normals   = attrib.normals;
    if (normals.empty()) normals = {0.0f, 1.0f, 0.0f};

    for (const auto& shape : shapes)
        for (const auto& index : shape.mesh.indices) {
            positionIndices.push_back(static_cast<uint32_t>(index.vertex_index));
            normalIndices.push_back(index.normal_index >= 0 ? static_cast<uint32_t>(index.normal_index) : 0);
        }

    std::cout << "Loaded model: " << positions.size() / 3 << " positions, "
              << normals.size() / 3 << " normals, "
              << positionIndices.size() / 3 << " triangles" << std::endl;
    return true;
}

// Overload 3 : positions + normales + texcoords + indices separes
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

    positions = attrib.vertices;
    normals   = attrib.normals;
    texcoords = attrib.texcoords;

    if (normals.empty())   normals   = {0.0f, 1.0f, 0.0f};
    if (texcoords.empty()) texcoords = {0.0f, 0.0f};

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

            float p0x = attrib.vertices[3*idx0.vertex_index+0], p0y = attrib.vertices[3*idx0.vertex_index+1], p0z = attrib.vertices[3*idx0.vertex_index+2];
            float p1x = attrib.vertices[3*idx1.vertex_index+0], p1y = attrib.vertices[3*idx1.vertex_index+1], p1z = attrib.vertices[3*idx1.vertex_index+2];
            float p2x = attrib.vertices[3*idx2.vertex_index+0], p2y = attrib.vertices[3*idx2.vertex_index+1], p2z = attrib.vertices[3*idx2.vertex_index+2];

            float uv0x=0,uv0y=0, uv1x=0,uv1y=0, uv2x=0,uv2y=0;
            if (idx0.texcoord_index>=0){uv0x=attrib.texcoords[2*idx0.texcoord_index];uv0y=attrib.texcoords[2*idx0.texcoord_index+1];}
            if (idx1.texcoord_index>=0){uv1x=attrib.texcoords[2*idx1.texcoord_index];uv1y=attrib.texcoords[2*idx1.texcoord_index+1];}
            if (idx2.texcoord_index>=0){uv2x=attrib.texcoords[2*idx2.texcoord_index];uv2y=attrib.texcoords[2*idx2.texcoord_index+1];}

            float e1x=p1x-p0x, e1y=p1y-p0y, e1z=p1z-p0z;
            float e2x=p2x-p0x, e2y=p2y-p0y, e2z=p2z-p0z;
            float du1=uv1x-uv0x, dv1=uv1y-uv0y;
            float du2=uv2x-uv0x, dv2=uv2y-uv0y;
            float denom = du1*dv2 - du2*dv1;
            float inv = (std::abs(denom)>1e-6f) ? 1.0f/denom : 0.0f;
            float tx=inv*(dv2*e1x-dv1*e2x), ty=inv*(dv2*e1y-dv1*e2y), tz=inv*(dv2*e1z-dv1*e2z);
            float len=std::sqrt(tx*tx+ty*ty+tz*tz);
            if(len>1e-6f){tx/=len;ty/=len;tz/=len;}else{tx=1;ty=0;tz=0;}

            for(size_t v=0;v<3;v++){
                tinyobj::index_t idx=shape.mesh.indices[indexOffset+v];
                if(idx.normal_index>=0){
                    size_t ni=static_cast<size_t>(idx.normal_index);
                    mesh.tangents[3*ni+0]=tx; mesh.tangents[3*ni+1]=ty; mesh.tangents[3*ni+2]=tz;
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
        if (!reader.Error().empty())
            std::cerr << "TinyObjReader error: " << reader.Error() << std::endl;
        return false;
    }
    if (!reader.Warning().empty())
        std::cout << "TinyObjReader warning: " << reader.Warning() << std::endl;

    const auto& attrib      = reader.GetAttrib();
    const auto& shapes      = reader.GetShapes();
    const auto& objMaterials = reader.GetMaterials();

    scene.clear();

    for (const auto& objMat : objMaterials) {
        Material mat;
        mat.name = objMat.name;
        for (int i=0;i<3;i++){ mat.ambient[i]=objMat.ambient[i]; mat.diffuse[i]=objMat.diffuse[i]; mat.specular[i]=objMat.specular[i]; mat.emission[i]=objMat.emission[i]; }
        mat.shininess = objMat.shininess;
        mat.opacity   = objMat.dissolve;
        mat.diffuseTexture  = objMat.diffuse_texname;
        mat.normalTexture   = objMat.bump_texname;
        mat.specularTexture = objMat.specular_texname;
        scene.addMaterial(mat);
    }

    for (const auto& shape : shapes) {
        Mesh mesh;
        mesh.name      = shape.name;
        mesh.positions = attrib.vertices;
        mesh.normals   = attrib.normals;
        mesh.texcoords = attrib.texcoords;
        if (mesh.normals.empty())   mesh.normals   = {0.0f,1.0f,0.0f};
        if (mesh.texcoords.empty()) mesh.texcoords = {0.0f,0.0f};

        for (const auto& index : shape.mesh.indices) {
            mesh.positionIndices.push_back(static_cast<uint32_t>(index.vertex_index));
            mesh.normalIndices.push_back(index.normal_index>=0 ? static_cast<uint32_t>(index.normal_index) : 0);
            mesh.texcoordIndices.push_back(index.texcoord_index>=0 ? static_cast<uint32_t>(index.texcoord_index) : 0);
        }

        if (!shape.mesh.material_ids.empty() && shape.mesh.material_ids[0]>=0)
            mesh.materialIndex = shape.mesh.material_ids[0];

        computeMeshTangents(mesh, attrib, shape);
        scene.addMesh(mesh);
    }

    std::cout << "Loaded scene: " << scene.getMeshCount() << " meshes, "
              << scene.getMaterialCount() << " materials" << std::endl;
    return true;
}
