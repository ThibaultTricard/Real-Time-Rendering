// TODO : Ecrire le vertex shader pour le rendu texture.
//
// Ce shader doit :
//   - Activer l'extension GL_EXT_scalar_block_layout (:require)
//   - Declarer six storage buffers en lecture seule :
//       binding 0 : positions       (layout(scalar) vec3[])
//       binding 1 : normals         (layout(scalar) vec3[])
//       binding 2 : texcoords       (layout(scalar) vec2[])
//       binding 3 : positionIndices (uint[])
//       binding 4 : normalIndices   (uint[])
//       binding 5 : texcoordIndices (uint[])
//   IMPORTANT : utiliser layout(scalar,...) pour les buffers vec3/vec2,
//   sinon std430 ajoute du padding (16 octets par vec3 au lieu de 12).
//   - Declarer un uniform buffer a binding 6 contenant une mat4 (viewProj)
//   - Declarer un push constant contenant une mat4 (model)
//   - Lire les indices via positionIndices[gl_VertexIndex], etc.
//   - Calculer gl_Position = viewProj * model * vec4(position, 1.0)
//   - Transmettre les coordonnees de texture (vec2) et la normale (vec3)
//     au fragment shader
//   - IMPORTANT : retourner le V des UV : fragUV = vec2(uv.x, 1.0 - uv.y)
//     (format OBJ : V=0 en bas ; Vulkan : V=0 en haut)
