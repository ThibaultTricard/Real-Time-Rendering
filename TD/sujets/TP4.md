# TP4 — Textures et Mipmaps

## Objectifs

- Charger une image PNG et l'uploader sur le GPU comme texture 2D
- Echantillonner une texture dans un fragment shader avec un **combined image sampler**
- Comprendre l'**aliasing** dû à l'absence de mipmaps et générer des mipmaps par blits GPU
- Utiliser `dFdx`/`dFdy` et comprendre pourquoi `textureGrad` diffère de `textureLod`

---

## Concepts clés

### Textures en Vulkan : Image, ImageView, Sampler

En Vulkan, une texture est représentée par trois objets distincts :

| Objet        | Rôle                                                                                 |
|--------------|--------------------------------------------------------------------------------------|
| `Image`      | Données brutes de l'image en mémoire GPU (pixels, niveaux de mipmap...)              |
| `ImageView`  | Vue sur l'image : indique le type (2D, cubemap...), le format, la plage de niveaux   |
| `Sampler`    | Paramètres de filtrage et d'adressage (linéaire/nearest, clamp/repeat, mipmaps...)   |

LavaCake fournit des classes correspondantes :

```cpp
// Image depuis des données CPU (uploade automatiquement sur le GPU)
LavaCake::Image diffuseTexture(device, pixelData,
    width, height, 1,
    vk::Format::eR8G8B8A8Srgb,
    vk::ImageUsageFlagBits::eSampled);
// Le constructeur effectue le transfert et la transition vers eShaderReadOnlyOptimal.

// Vue par défaut : 2D, couleur, tous les niveaux de mipmap
LavaCake::ImageView diffuseView(diffuseTexture);

// Sampler par défaut : filtrage linéaire, repeat, mipmaps activés
LavaCake::Sampler diffuseSampler(device);
```

> **Note :** `LavaCake::Image` accepte un paramètre `mipLevels` (8e argument, défaut = 1) pour créer une image avec plusieurs niveaux de mipmap. Si `mipLevels > 1`, seul le niveau 0 est rempli par le constructeur avec données : les niveaux suivants doivent être générés manuellement.

---

### Chargement d'une image avec stb_image

`stb_image` est une bibliothèque en-tête unique (`/usr/include/stb/stb_image.h`) qui décode une image depuis le disque et retourne un tableau de pixels en mémoire CPU. La fonction principale est :

```cpp
stbi_uc* stbi_load(const char* filename,
                   int* width,             // rempli par stb avec la largeur de l'image
                   int* height,            // rempli par stb avec la hauteur de l'image
                   int* channels_in_file,  // canaux originaux de l'image ; nullptr si inutile
                   int desired_channels);  // canaux forces en sortie (ex. STBI_rgb_alpha = 4)
```

`width` et `height` sont des paramètres de sortie : on passe l'adresse de variables locales, et stb les remplit avec les dimensions de l'image. La fonction retourne `nullptr` en cas d'échec. Les pixels renvoyés sont alloués par stb et doivent être libérés avec `stbi_image_free(pixels)` après usage. `stbi_uc` est un alias de `uint8_t`.

> **`STBI_rgb_alpha`** force la sortie à 4 canaux (RGBA) quelle que soit l'image source. Comme on impose le format, le paramètre `channels_in_file` ne nous est pas utile : on passe `nullptr`.

---

### Combined Image Sampler : déclaration et binding

Un **combined image sampler** regroupe une `ImageView` et un `Sampler` en un seul descriptor. C'est le type le plus courant pour les textures de rendu.

**Dans le DescriptorSetLayout :**
```cpp
.addCombinedImageSampler(binding, vk::ShaderStageFlagBits::eFragment)
```

**Dans le DescriptorPool :**
```cpp
.addCombinedImageSamplers(count)
```

**Dans l'updater :**
```cpp
.bindImage(binding, imageView, sampler)
// Le layout par défaut est eShaderReadOnlyOptimal.
```

**Dans le shader GLSL :**
```glsl
layout(set = 0, binding = 7) uniform sampler2D diffuseTexture;

// Echantillonnage :
vec4 color = texture(diffuseTexture, uv);
```

---

### Aliasing et Mipmaps

#### Pourquoi l'aliasing apparaît-il ?

Lorsqu'un objet est vu de loin, plusieurs texels de la texture sont regroupés dans un seul pixel à l'écran. Si l'on n'échantillonne qu'un seul texel (niveau 0), des motifs de Moiré ou de scintillement apparaissent lors des mouvements : c'est l'**aliasing**.

#### Les mipmaps

Les **mipmaps** sont une pyramide de versions pré-filtrées de la texture, chaque niveau étant deux fois plus petit que le précédent. Le matériel GPU choisit automatiquement le niveau adapté à la distance de rendu (LOD — *Level of Detail*).

Nombre de niveaux de mipmap pour une image de dimensions `w × h` :

```cpp
uint32_t mipLevels =
    static_cast<uint32_t>(std::floor(std::log2(std::max(w, h)))) + 1;
```

Pour une image 1024×1024 : 11 niveaux (1024, 512, 256, ..., 2, 1).

#### Génération des mipmaps par blits

Vulkan ne génère pas les mipmaps automatiquement. Il faut les générer par une suite de blits, chaque niveau étant réduit à partir du niveau précédent :

```
Niveau 0 (1024×1024) -blit-> Niveau 1 (512×512) -blit-> Niveau 2 (256×256) -blit-> ...
```

Pour chaque blit, des **barrières mémoire** (`vk::ImageMemoryBarrier`) sont nécessaires pour transitionner le layout de chaque niveau au bon moment.

#### Spec : vk::ImageMemoryBarrier et pipelineBarrier

```cpp
vk::ImageMemoryBarrier barrier{};
barrier.image               = /* vk::Image cible */;
barrier.oldLayout           = /* layout actuel */;
barrier.newLayout           = /* layout souhaité */;
barrier.srcAccessMask       = /* acces a attendre avant la barriere */;
barrier.dstAccessMask       = /* acces autorises apres la barriere */;
barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // pas de transfert de propriete
barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
barrier.subresourceRange    = {
    vk::ImageAspectFlagBits::eColor,
    baseMipLevel,  // premier niveau concerné
    levelCount,    // nombre de niveaux
    0, 1           // baseArrayLayer, layerCount
};
```

Envoi via :

```cpp
cmd.pipelineBarrier(
    srcStageMask,  // étape productrice (ex. eTransfer, eTopOfPipe)
    dstStageMask,  // étape consommatrice (ex. eTransfer, eFragmentShader)
    {},            // dependencyFlags
    {},            // memory barriers (non utilisé ici)
    {},            // buffer barriers (non utilisé ici)
    { barrier }    // image barriers
);
```

#### Spec : vk::ImageBlit et blitImage

```cpp
vk::ImageBlit blit{};
blit.srcSubresource = { vk::ImageAspectFlagBits::eColor, srcMipLevel, 0, 1 };
blit.srcOffsets[0]  = { 0, 0, 0 };
blit.srcOffsets[1]  = { srcWidth, srcHeight, 1 }; // coin bas-droit de la region source
blit.dstSubresource = { vk::ImageAspectFlagBits::eColor, dstMipLevel, 0, 1 };
blit.dstOffsets[0]  = { 0, 0, 0 };
blit.dstOffsets[1]  = { dstWidth, dstHeight, 1 };
```

Envoi via :

```cpp
cmd.blitImage(
    srcImage, srcLayout,
    dstImage, dstLayout,
    { blit },
    vk::Filter::eLinear  // ou eNearest
);
```

---

### Dérivées partielles : dFdx et dFdy

En GLSL, `dFdx` et `dFdy` calculent la **dérivée d'une valeur par rapport à la position écran** d'un fragment à l'autre. Pour les coordonnées de texture `uv` :

```glsl
vec2 duvdx = dFdx(uv);  // "de combien varient les UV vers la droite ?"
vec2 duvdy = dFdy(uv);  // "de combien varient les UV vers le bas ?"
```

Ces gradients permettent de sélectionner le bon niveau de mipmap avec `textureGrad`, qui prend le contrôle explicite du LOD :

```glsl
vec4 color = textureGrad(diffuseTexture, uv, duvdx, duvdy);
```

Avantage : même résultat que `texture()` sur une géométrie normale, mais garantit le comportement correct dans tous les cas (shaders de calcul, effets de post-traitement...).

---

---

## Construction

```bash
cd realtimerendering-students
cmake -B build
cmake --build build --target TP4_exo1
./build/TP4_exo1
```

---

## Exercice 1 — Affichage d'un modèle OBJ texturé

L'objectif est d'afficher le modèle `Michelle.obj` avec sa texture diffuse `Ch03_1001_Diffuse.png`. Le chargement OBJ, les six storage buffers et la structure de rendu sont fournis dans le squelette.

### Étapes

**TODO 1 — Charger la texture**

Utilisez `stbi_load` avec `STBI_rgb_alpha` pour forcer 4 canaux. Copiez les pixels dans un `std::vector<uint8_t>`. Créez `LavaCake::Image` avec le format `eR8G8B8A8Srgb`, puis `LavaCake::ImageView` et `LavaCake::Sampler` avec les valeurs par défaut.

**TODO 2 — Descriptor set layout**

Créez le layout avec 8 bindings :
- Bindings 0–5 : storage buffers vertex (positions, normals, texcoords, posIndices, nrmIndices, texIndices)
- Binding 6 : uniform buffer vertex (viewProj)
- Binding 7 : combined image sampler fragment (texture diffuse)

**TODO 3 — Uniform buffer viewProj**

Calculez `viewProj = proj * view` (caméra fixe). Créez `LavaCake::UniformBuffer`, ajoutez la variable, appelez `end()`.

**TODO 4 — Pool, allocation et mise à jour du descriptor set**

Créez le pool avec `addStorageBuffers(6)`, `addUniformBuffers(1)`, `addCombinedImageSamplers(1)`. Allouez le descriptor set. Bindez les 8 ressources avec `DescriptorSetUpdater`, dont la texture au binding 7 avec `.bindImage(7, diffuseView, diffuseSampler)`.

**TODO 5 — Boucle de rendu**

Calculez une matrice `model` rotative, uploadez l'UBO, créez le `DynamicRenderingContext`, bindez le pipeline et le descriptor set, envoyez la matrice `model` via `pushConstants`, dessinez `vertexCount` sommets.

### Shaders GLSL

**`shaders/TP4/obj_tex.vert`** : déclarez les 6 storage buffers, l'UBO `viewProj` (binding 6) et le push constant `model`. Calculez `gl_Position = viewProj * model * vec4(pos, 1.0)`. Transmettez les UV et la normale au fragment shader. Pensez à retourner le V : `fragUV = vec2(uv.x, 1.0 - uv.y)` (le format OBJ place V=0 en bas, Vulkan en haut).

> **Important — layout(scalar) :** Sans ce qualificateur, `vec3[]` utilise le layout std430 qui aligne chaque élément sur 16 octets au lieu de 12. Les positions lues seraient complètement fausses. Utilisez :
> ```glsl
> #extension GL_EXT_scalar_block_layout : require
> layout(scalar, set = 0, binding = 0) readonly buffer Positions { vec3 positions[]; };
> layout(scalar, set = 0, binding = 1) readonly buffer Normals   { vec3 normals[]; };
> layout(scalar, set = 0, binding = 2) readonly buffer TexCoords { vec2 texcoords[]; };
> layout(        set = 0, binding = 3) readonly buffer PosIndices { uint posIndices[]; };
> // etc.
> ```

**`shaders/TP4/obj_tex.frag`** : déclarez `sampler2D` au binding 7. Echantillonnez avec `texture(diffuseTexture, uv)`. Appliquez un éclairage de Lambert sur la couleur obtenue.

---

## Exercice 2 — Génération de mipmaps

La caméra oscillante fournie dans le squelette met en évidence l'aliasing : sans mipmaps, la texture scintille lorsque l'objet est vu de loin.

### Étapes

**TODO 1 — Créer l'image et uploader le niveau 0**

Calculez `mipLevels` avec la formule `floor(log2(max(w, h))) + 1`. Créez un `LavaCake::Image` avec le constructeur (`pixelData`), le format `eR8G8B8A8Srgb`, les usages `eSampled | eTransferSrc`, et `mipLevels` niveaux. LavaCake ajoute `eTransferDst` automatiquement, uploade le niveau 0 et transite tous les niveaux vers `eShaderReadOnlyOptimal`. Créez ensuite `ImageView` et `Sampler`.

**TODO 2 — Générer les niveaux de mipmap par blits**

Créez un `LavaCake::CommandBuffer` (mode one-shot). Écrivez :

**Étape 0** — Retransitionner tous les niveaux vers `eTransferDstOptimal` :  
Une seule barrier `eShaderReadOnlyOptimal` → `eTransferDstOptimal` avec `levelCount = mipLevels`  
(`srcAccess=eShaderRead`, `dstAccess=eTransferWrite`, stages `eFragmentShader → eTransfer`)

Puis pour chaque niveau `i` de 1 à `mipLevels-1` :

1. **Barrier** niveau `i-1` : `eTransferDstOptimal` → `eTransferSrcOptimal`  
   (`srcAccess=eTransferWrite`, `dstAccess=eTransferRead`, stages `eTransfer → eTransfer`)

2. **Blit** du niveau `i-1` vers le niveau `i` avec filtre linéaire  
   (`srcOffsets[1] = { max(w >> (i-1), 1), max(h >> (i-1), 1), 1 }`,  
    `dstOffsets[1] = { max(w >> i, 1), max(h >> i, 1), 1 }`)

3. **Barrier** niveau `i-1` : `eTransferSrcOptimal` → `eShaderReadOnlyOptimal`  
   (`srcAccess=eTransferRead`, `dstAccess=eShaderRead`, stages `eTransfer → eFragmentShader`)

Après la boucle, transitionner le dernier niveau `mipLevels-1` : `eTransferDstOptimal` → `eShaderReadOnlyOptimal`. Soumettre et attendre avec `device.getAnyQueue()`.

**TODO 3 — Shader : remplacer `texture` par `textureGrad`**

Dans `shaders/TP4/obj_mip.frag`, calculez `dFdx(uv)` et `dFdy(uv)`, puis utilisez `textureGrad(diffuseTexture, uv, duvdx, duvdy)`. Observez la réduction du scintillement.

---

## Exercice 3 — Visualisation du moiré sur un plan

L'objectif est de confirmer visuellement l'effet des mipmaps sur un cas simple : un plan texturé avec un damier. Le motif régulier du damier rend le moiré immédiatement visible dès que la caméra s'éloigne.

Le plan et la texture damier sont fournis dans le squelette. La caméra oscille automatiquement entre une position proche et une position lointaine.

### Étapes

**TODO 1 — Créer la texture avec mipmaps**

Reprenez le même schéma qu'en exercice 2 :
- Calculez `mipLevels` avec `mipLevelCount(texW, texH)` (fourni dans `MipmapHelper.hpp`)
- Créez l'image sans données avec les usages `eSampled | eTransferDst | eTransferSrc` et `mipLevels` niveaux
- Uploadez les pixels du damier dans le niveau 0 via un staging buffer

**TODO 2 — Générer les mipmaps**

Appelez `generateMipmaps((vk::Image)texture, texW, texH, mipLevels, device)` après l'upload.

Décommentez ensuite `.bindImage(7, texView, texSampler)` dans l'updater.

**TODO 3 — Observer l'effet dans le shader**

Dans `shaders/TP4/obj_mip.frag`, comparez les deux lignes suivantes et observez la différence :

```glsl
vec4 texColor = texture(diffuseTex, fragUV);          // mipmaps actifs — pas de moire
// vec4 texColor = textureLod(diffuseTex, fragUV, 0.0); // force le niveau 0 — moire visible
```

> **Pourquoi `textureGrad(uv, dFdx(uv), dFdy(uv))` est-il identique à `texture(uv)` ?**
> Le GPU calcule déjà ces mêmes dérivées en interne pour sélectionner le LOD. Passer `dFdx`/`dFdy` explicitement ne change donc rien. `textureGrad` devient utile seulement quand on veut **modifier** les gradients (par exemple, pour biaiser le LOD ou dans un compute shader où les dérivées automatiques ne sont pas disponibles).
