# TP6 — Chargement de scènes glTF 2.0

## Objectifs

- Comprendre la structure du format **glTF 2.0** et son positionnement par rapport à OBJ
- Extraire la géométrie d'un fichier `.glb` via le système d'**accessors**
- Tirer parti de l'index unique de glTF pour simplifier le **vertex pulling**
- Lire les paramètres de **matériaux PBR** directement depuis le fichier
- Parcourir la **hiérarchie de scène** (nodes, transforms) pour rendre plusieurs objets

---

## Contexte : OBJ vs glTF 2.0

| | OBJ | glTF 2.0 |
|---|---|---|
| Contenu | Mesh(es) uniquement | Scène complète (nodes, caméras, lumières) |
| Matériaux | **Phong** (ambient, diffuse, specular, shininess) | **PBR metallic-roughness** (baseColor, metallic, roughness) |
| Indices | Séparés par attribut (positions, normales…) | **Un seul** index buffer partagé entre tous les attributs |
| Format binaire | Texte | JSON + binaire (`.glb` = tout-en-un) |
| Standard | Années 90, simple | Khronos 2017, industriel |

### L'index unique et le vertex pulling

En OBJ, chaque attribut peut avoir ses propres indices (exercice 3 du TP3). glTF impose un **index unique** partagé par toutes les données d'un sommet. Aux seams UV ou arêtes vives, le sommet est dupliqué dans **tous** les buffers d'attributs.

Conséquence directe sur le vertex pulling : l'accès aux données est **plus simple** qu'avec l'OBJ.

```glsl
// OBJ (TP3 exercice 3) — deux niveaux d'indirection
uint posIdx = posIndices[gl_VertexIndex];
uint nrmIdx = normalIndices[gl_VertexIndex];
vec3 pos    = positions[posIdx];
vec3 nor    = normals[nrmIdx];

// glTF — accès direct avec un seul index
vec3 pos = positions[gl_VertexIndex];
vec3 nor = normals[gl_VertexIndex];
vec2 uv  = texcoords[gl_VertexIndex];
```

---

## Structure d'un fichier glTF

Un fichier glTF organise ses données en couches successives :

```
Model
└── Scene (liste de nodes racines)
    └── Node (transform TRS + référence à un Mesh + enfants)
        └── Mesh
            └── Primitive (attributs + indices + matériau)
                ├── Accessor "POSITION"  ──┐
                ├── Accessor "NORMAL"    ──┤──> BufferView ──> Buffer (données binaires)
                ├── Accessor "TEXCOORD_0"─┘
                └── Accessor indices
```

### Accessor, BufferView, Buffer

Un **Accessor** décrit comment interpréter une tranche de données binaires :
- `type` : `VEC3`, `VEC2`, `SCALAR`…
- `componentType` : `FLOAT`, `UNSIGNED_INT`, `UNSIGNED_SHORT`…
- `count` : nombre d'éléments

Un **BufferView** est une fenêtre sur un **Buffer** (offset + longueur en octets). Un **Buffer** contient les données binaires brutes.

```cpp
// Récupérer un pointeur typé vers les données d'un accessor
auto getData = [&](int accessorIdx) -> const unsigned char* {
    const auto& acc = model.accessors[accessorIdx];
    const auto& bv  = model.bufferViews[acc.bufferView];
    return model.buffers[bv.buffer].data.data()
         + bv.byteOffset + acc.byteOffset;
};

// Positions (VEC3 float)
const float* positions = reinterpret_cast<const float*>(
    getData(primitive.attributes.at("POSITION"))
);
size_t vertexCount = model.accessors[primitive.attributes.at("POSITION")].count;
```

### Types d'indices

glTF supporte trois types pour les indices. Il faut tester `componentType` pour convertir en `uint32_t` :

```cpp
std::vector<uint32_t> indices;
const auto& idxAcc = model.accessors[primitive.indices];
const unsigned char* idxData = getData(primitive.indices);

if (idxAcc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
    const uint16_t* src = reinterpret_cast<const uint16_t*>(idxData);
    indices.assign(src, src + idxAcc.count);
} else { // UNSIGNED_INT
    const uint32_t* src = reinterpret_cast<const uint32_t*>(idxData);
    indices.assign(src, src + idxAcc.count);
}
```

### Matériaux PBR

```cpp
const tinygltf::Material& mat = model.materials[primitive.material];
const auto& pbr = mat.pbrMetallicRoughness;

// Facteurs scalaires (vec4 pour baseColor, float pour les autres)
glm::vec4 baseColorFactor = glm::make_vec4(pbr.baseColorFactor.data());
float     metallicFactor  = static_cast<float>(pbr.metallicFactor);
float     roughnessFactor = static_cast<float>(pbr.roughnessFactor);

// Index de texture (-1 si absent)
int baseColorTexIdx      = pbr.baseColorTexture.index;       // RGB
int metallicRoughnessIdx = pbr.metallicRoughnessTexture.index; // B=metallic, G=roughness
int normalMapIdx         = mat.normalTexture.index;          // RGB → tangent-space normal
```

### Hiérarchie de scène

```cpp
// Parcours récursif des nodes
void traverseNode(const tinygltf::Model& model, int nodeIdx,
                  const glm::mat4& parentTransform)
{
    const tinygltf::Node& node = model.nodes[nodeIdx];

    // Récupérer le transform local (matrice directe ou TRS)
    glm::mat4 localTransform = glm::mat4(1.0f);
    if (!node.matrix.empty()) {
        localTransform = glm::make_mat4(node.matrix.data());
    } else {
        if (!node.translation.empty())
            localTransform = glm::translate(localTransform, glm::make_vec3(node.translation.data()));
        if (!node.rotation.empty()) {
            glm::quat q = glm::make_quat(node.rotation.data());
            localTransform *= glm::mat4_cast(q);
        }
        if (!node.scale.empty())
            localTransform = glm::scale(localTransform, glm::make_vec3(node.scale.data()));
    }

    glm::mat4 worldTransform = parentTransform * localTransform;

    if (node.mesh >= 0) {
        // Rendre le mesh avec worldTransform
    }

    for (int child : node.children)
        traverseNode(model, child, worldTransform);
}

// Point d'entrée : parcourir la scène par défaut
const tinygltf::Scene& scene = model.scenes[model.defaultScene];
for (int rootNode : scene.nodes)
    traverseNode(model, rootNode, glm::mat4(1.0f));
```

---

## Compilation et exécution

```bash
cd realtimerendering-students
cmake -B build
cmake --build build --target TP6_exercice1
./build/TP6_exercice1
```

---

## Exercice 1 — Charger la géométrie

L'objectif est d'afficher un modèle `.glb` avec une couleur uniforme, en extrayant positions et indices depuis les accessors glTF.

### Chargement du fichier

```cpp
#include "tiny_gltf.h"

tinygltf::Model    gltfModel;
tinygltf::TinyGLTF loader;
std::string        err, warn;

bool ok = loader.LoadBinaryFromFile(&gltfModel, &err, &warn,
                                    root + "models/DamagedHelmet.glb");
if (!ok) throw std::runtime_error("glTF load failed: " + err);

const tinygltf::Primitive& prim = gltfModel.meshes[0].primitives[0];
```

### Étapes

**TODO 1 — Extraire les positions**

Utilisez le lambda `getData` de la section concepts pour récupérer un pointeur `const float*` vers les positions (`POSITION`, VEC3 float). Copiez les données dans un `std::vector<float>`.

**TODO 2 — Extraire les indices**

Récupérez l'accessor d'indices (`primitive.indices`). Testez le `componentType` pour convertir en `std::vector<uint32_t>` (voir la section sur les types d'indices).

**TODO 3 — Buffers GPU**

Créez deux storage buffers : `posBuffer` (positions) et `idxBuffer` (indices), comme en TP3.

**TODO 4 — Uniform Buffer MVP**

Même logique que TP2/TP3 : view + projection + correction axe Y Vulkan, `UniformBuffer` contenant `mvp`.

**TODO 5 — Descriptor Set**

Un descriptor set avec 2 storage buffers (positions + indices) et 1 uniform buffer.

**TODO 6 — Pipeline**

Créez `shaders/TP6/flat.vert` et `shaders/TP6/flat.frag`. Le vertex shader lit `positions[indices[gl_VertexIndex]]` et applique la MVP. Le fragment shader sort une couleur constante.

**TODO (boucle)** — Mettre à jour la MVP à chaque frame, dessiner `indices.size()` sommets.

---

## Exercice 2 — Normales et éclairage

Partez de l'exercice 1. Ajoutez les normales issues du fichier glTF et appliquez l'éclairage de Lambert (TP3). Observez que, contrairement à l'OBJ, un seul index suffit pour lire simultanément position et normale.

### Étapes

**TODO 1 — Extraire les normales**

Même démarche que pour les positions, avec l'attribut `"NORMAL"` (VEC3 float). La taille du vecteur est identique à celui des positions (même `vertexCount`).

**TODO 2 — Buffer GPU**

Ajoutez `nrmBuffer` au pipeline. Le vertex shader lira :

```glsl
vec3 pos    = positions[indices[gl_VertexIndex]];
vec3 normal = normals  [indices[gl_VertexIndex]];
```

**TODO 3 — Shader**

Écrivez `shaders/TP6/lambert.vert` en partant de `flat.vert`. Transmettez la normale interpolée au fragment shader. Réutilisez `shaders/TP3/obj.frag` pour l'éclairage de Lambert.

**TODO 4–5 — Descriptor Set, Pipeline**

Ajoutez le buffer de normales au descriptor set layout (binding supplémentaire).

### Comparaison avec TP3

Dans le vertex shader OBJ (TP3 exercice 3), vous lisiez :
```glsl
uint posIdx = posIndices[gl_VertexIndex];
uint nrmIdx = normalIndices[gl_VertexIndex];
```
Ici, un seul niveau d'indirection est nécessaire grâce à l'index unique glTF. La structure des données côté CPU est également plus simple à mettre en place.

---

## Exercice 3 — Matériaux PBR

Partez de l'exercice 2. Lisez les paramètres du matériau PBR depuis le fichier glTF et appliquez le BRDF GGX complet vu en TP5.

### Étapes

**TODO 1 — Extraire les coordonnées de texture**

Extrayez l'attribut `"TEXCOORD_0"` (VEC2 float). Ajoutez `uvBuffer` au pipeline.

**TODO 2 — Lire les paramètres matériau**

Depuis `model.materials[primitive.material].pbrMetallicRoughness`, récupérez :
- `baseColorFactor` (vec4)
- `metallicFactor` (float)
- `roughnessFactor` (float)

Transmettez ces valeurs au fragment shader dans un `UniformBuffer` :

```cpp
struct PbrParams {
    glm::vec4 baseColorFactor;
    float     metallicFactor;
    float     roughnessFactor;
};
```

**TODO 3 — Charger la base color texture**

Récupérez l'index de texture `pbr.baseColorTexture.index`. Extrayez l'image correspondante depuis les buffers glTF :

```cpp
const tinygltf::Texture& tex   = model.textures[texIdx];
const tinygltf::Image&   img   = model.images[tex.source];
// img.image  : données RGBA uint8
// img.width, img.height : dimensions
```

Uploadez l'image sur le GPU (même procédure que TP4).

**TODO 4 — Shader PBR**

Écrivez `shaders/TP6/pbr.frag` en partant du fragment shader du TP5. Remplacez les constantes codées en dur par les valeurs issues du `UniformBuffer` (`baseColorFactor`, `metallicFactor`, `roughnessFactor`). Ajoutez l'échantillonnage de la base color texture.

**TODO 5 — Descriptor Set, Pipeline**

Ajoutez le buffer UV, le `UniformBuffer` de paramètres PBR et le sampler de texture au descriptor set.

---

## Exercice 4 — Hiérarchie de scène

L'objectif est de parcourir la scène glTF et d'appliquer les transforms des nodes pour rendre correctement un fichier contenant plusieurs objets.

### Pourquoi une hiérarchie ?

Un fichier glTF peut décrire une scène entière : plusieurs meshes positionnés dans l'espace par des nodes, eux-mêmes agencés en arbre (parent/enfant). La matrice monde d'un node est le produit de son transform local par le transform monde de son parent.

$$M_{world}(n) = M_{world}(parent(n)) \times M_{local}(n)$$

### Étapes

**TODO 1 — Implémenter `traverseNode`**

Implémentez la fonction de parcours décrite dans la section concepts. Elle doit gérer les deux cas de transform (matrice directe vs TRS décomposé).

**TODO 2 — Collecter les draw calls**

Lors du parcours, pour chaque node possédant un mesh, enregistrez un couple `(worldTransform, primitive)` dans un vecteur de draw calls à exécuter.

**TODO 3 — Rendre avec push constants**

Transmettez la matrice monde de chaque objet via un push constant (comme en TP2 exercice 4). La matrice `viewProj` reste dans un uniform buffer partagé.

**TODO 4 — Point d'entrée de la scène**

```cpp
const tinygltf::Scene& scene = gltfModel.scenes[gltfModel.defaultScene];
for (int rootNode : scene.nodes)
    traverseNode(gltfModel, rootNode, glm::mat4(1.0f));
```

### Comparaison avec OBJ

Le format OBJ ne définit aucune hiérarchie de scène. Les positions des objets sont soit encodées dans la géométrie, soit gérées entièrement par l'application. glTF externalise cette information dans le fichier lui-même — le renderer n'a plus qu'à traverser l'arbre.
