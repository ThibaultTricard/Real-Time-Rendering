// TODO : Modifier cube.vert pour separer la matrice MVP en deux parties :
//
//   - Un uniform buffer a binding 3 contenant la matrice viewProj (view * projection)
//     Cette matrice est fixe pendant toute la boucle de rendu.
//
//   - Un push constant contenant la matrice model
//     Cette matrice change a chaque frame pour animer le cube.
//
//   La transformation finale devient : viewProj * model * vec4(position, 1.0)
