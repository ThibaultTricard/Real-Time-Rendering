// TODO : Adapter perFaceNormal.vert pour les normales par sommet.
//
// Les normales calculees en exercice 2 sont stockees par sommet unique
// (une normale par position unique, memes indices que les positions).
//
// Seule la lecture de la normale change :
//   - En per-face  : normals[gl_VertexIndex / 3]
//   - En per-sommet: normals[posIndices[gl_VertexIndex]]
//
// Le reste du shader (declaration des buffers, mvp, gl_Position) est identique.
