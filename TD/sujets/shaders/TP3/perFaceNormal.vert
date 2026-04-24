// TODO : Ecrire le vertex shader pour l'eclairage par face.
//
// Ce shader doit :
//   - Activer l'extension GL_EXT_scalar_block_layout
//   - Declarer trois storage buffers en lecture seule (scalar layout pour les vec3) :
//       binding 0 : positions  (vec3[])
//       binding 1 : normals    (vec3[]) -- une normale par triangle
//       binding 2 : posIndices (uint[])
//   - Declarer un uniform buffer a binding 3 contenant une mat4 (mvp)
//   - Lire l'indice de position : posIndices[gl_VertexIndex]
//   - Calculer gl_Position = mvp * vec4(positions[posIdx], 1.0)
//   - La normale de face correspond au triangle courant :
//       normals[gl_VertexIndex / 3]
//     (les trois sommets d'un meme triangle partagent la meme normale)
//   - Transmettre la normale au fragment shader
