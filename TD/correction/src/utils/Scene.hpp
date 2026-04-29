#pragma once

#include <vector>
#include <string>
#include <cstdint>

struct Material {
    std::string name;

    float ambient[3]  = {1.0f, 1.0f, 1.0f};
    float diffuse[3]  = {0.8f, 0.8f, 0.8f};
    float specular[3] = {0.5f, 0.5f, 0.5f};
    float emission[3] = {0.0f, 0.0f, 0.0f};

    float shininess = 32.0f;
    float opacity   = 1.0f;

    std::string diffuseTexture;
    std::string normalTexture;
    std::string specularTexture;
};

struct Mesh {
    std::string name;

    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> texcoords;
    std::vector<float> tangents;

    std::vector<uint32_t> positionIndices;
    std::vector<uint32_t> normalIndices;
    std::vector<uint32_t> texcoordIndices;
    std::vector<uint32_t> tangentIndices;

    int materialIndex = -1;
};

class Scene {
public:
    std::vector<Mesh> meshes;
    std::vector<Material> materials;

    void clear() {
        meshes.clear();
        materials.clear();
    }

    size_t getMeshCount() const { return meshes.size(); }
    size_t getMaterialCount() const { return materials.size(); }

    Mesh& getMesh(size_t index) { return meshes[index]; }
    const Mesh& getMesh(size_t index) const { return meshes[index]; }

    Material& getMaterial(size_t index) { return materials[index]; }
    const Material& getMaterial(size_t index) const { return materials[index]; }

    void addMesh(const Mesh& mesh) { meshes.push_back(mesh); }
    void addMaterial(const Material& material) { materials.push_back(material); }
};
