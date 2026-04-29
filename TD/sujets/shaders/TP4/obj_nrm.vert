// TODO : Vertex shader pour le rendu avec normal mapping.
//
// Identique a obj_tex.vert, mais transmettre en plus la position dans l'espace
// monde (vec3, location 2) au fragment shader, afin de pouvoir y calculer
// la matrice TBN par derivees ecran.
//
// Rappel des sorties attendues :
//   layout(location = 0) out vec2 outUV;
//   layout(location = 1) out vec3 outNormal;   // normale en espace monde
//   layout(location = 2) out vec3 outWorldPos; // position en espace monde
