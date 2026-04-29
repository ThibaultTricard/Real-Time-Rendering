// TODO : Ecrire le fragment shader pour le rendu texture.
//
// Ce shader doit :
//   - Recevoir les coordonnees de texture (vec2, location 0) depuis le vertex shader
//   - Recevoir la normale interpolee (vec3, location 1) depuis le vertex shader
//   - Declarer un combined image sampler a binding 7 :
//       layout(set = 0, binding = 7) uniform sampler2D diffuseTexture;
//   - Echantillonner la texture : vec4 color = texture(diffuseTexture, uv);
//   - Appliquer un eclairage diffus de Lambert sur la couleur de la texture
//   - Ecrire la couleur finale dans outColor
