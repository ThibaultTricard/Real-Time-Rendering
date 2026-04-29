// TODO : Fragment shader pour le rendu avec mipmaps.
//
// Reprendre obj_tex.frag, puis :
//
// TODO 4 : Remplacer l'appel texture() par textureGrad() pour utiliser
//          explicitement les derivees partielles dans le calcul du LOD.
//
//   Les derivees partielles indiquent la vitesse de variation des UV
//   par rapport aux coordonnees ecran (x et y) :
//     vec2 duvdx = dFdx(uv);   // variation de uv vers la droite
//     vec2 duvdy = dFdy(uv);   // variation de uv vers le bas
//
//   Echantillonnage avec LOD explicite :
//     vec4 color = textureGrad(diffuseTexture, uv, duvdx, duvdy);
//
//   Sans mipmaps, de grands gradients (zone tres zoomee) produisent de l'aliasing.
//   Avec mipmaps, le sampler selectionne automatiquement le bon niveau.
