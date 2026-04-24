// Vertex shader : normales lues directement depuis le fichier OBJ
//
// Les normales OBJ ont des indices independants des indices de position.
// Pour le sommet gl_VertexIndex :
//   - la position est  positions[posIndices[gl_VertexIndex]]
//   - la normale est   normals[normalIndices[gl_VertexIndex]]
//
// Bindings :
//   binding 0 : positions      (vec3[])
//   binding 1 : normals        (vec3[])
//   binding 2 : posIndices     (uint[])
//   binding 3 : normalIndices  (uint[])
//   binding 4 : ubo            (mat4 mvp)
//
// Sortie :
//   location 0 : fragNormal (vec3)

// TODO : ecrire le shader depuis zero
