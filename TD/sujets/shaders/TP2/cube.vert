// TODO : Ecrire le vertex shader depuis zero.
//
// Ce shader doit :
//   - Activer l'extension GL_EXT_scalar_block_layout
//   - Declarer trois storage buffers en lecture seule :
//       binding 0 : positions (vec3[])
//       binding 1 : couleurs  (vec3[])
//       binding 2 : indices   (uint[])
//   - Declarer un uniform buffer a binding 3 contenant une mat4 (mvp)
//   - Lire l'indice du sommet courant via indices[gl_VertexIndex]
//   - Calculer gl_Position = mvp * vec4(positions[index], 1.0)
//   - Transmettre la couleur au fragment shader
