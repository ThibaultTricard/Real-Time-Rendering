// TODO : Ecrire le vertex shader de vertex pulling depuis zero.
//
// Ce shader doit :
//   - Activer l'extension GL_EXT_scalar_block_layout
//   - Declarer deux storage buffers en lecture seule :
//       binding 0 : positions (vec2[])
//       binding 1 : couleurs  (vec3[])
//   - Lire la position et la couleur du sommet courant via gl_VertexIndex
//   - Ecrire gl_Position et transmettre la couleur au fragment shader
