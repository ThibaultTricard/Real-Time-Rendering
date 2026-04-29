// TODO : Fragment shader pour le rendu avec normal mapping.
//
// Reprendre obj_mip.frag, puis :
//
// TODO 3 : Decoder la normal map et construire la matrice TBN
//          en espace ecran (sans tangentes precalculees).
//
//   Etape A — Decoder le texel de la normal map (binding 8) :
//     layout(set = 0, binding = 8) uniform sampler2D normalMap;
//     vec3 nrmSample = texture(normalMap, uv).rgb * 2.0 - 1.0;
//     // Convertit [0,1] -> [-1,1]
//
//   Etape B — Calculer la matrice TBN par derivees ecran :
//     // dFdx/dFdy donnent la variation de la position monde et des UV
//     // d'un fragment a l'autre, permettant de reconstruire tangente et bitangente.
//     vec3 dposdx = dFdx(worldPos);
//     vec3 dposdy = dFdy(worldPos);
//     vec2 duvdx  = dFdx(uv);
//     vec2 duvdy  = dFdy(uv);
//
//     // Formule de Mikkelsen (reconstruction TBN sans attributs de sommet) :
//     vec3 N = normalize(normal);
//     vec3 T = normalize(dposdx * duvdy.y - dposdy * duvdx.y);
//     vec3 B = normalize(cross(N, T));
//     mat3 TBN = mat3(T, B, N);
//
//   Etape C — Appliquer la normale mappee :
//     vec3 mappedNormal = normalize(TBN * nrmSample);
//
//   Etape D — Utiliser mappedNormal dans le calcul de Lambert au lieu de normal.
