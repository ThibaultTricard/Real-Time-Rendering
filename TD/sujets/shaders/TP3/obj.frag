// TODO : Ecrire le fragment shader depuis zero.
//
// Ce shader doit :
//   - Recevoir la normale interpolee depuis le vertex shader (vec3, location 0)
//   - Implementer un eclairage diffus simple (modele de Lambert) :
//       - Definir une direction de lumiere fixe (ex : normalize(vec3(1, 2, 1)))
//       - Calculer l'intensite diffuse : max(dot(normalize(normal), lightDir), 0.0)
//       - Combiner avec une composante ambiante pour eviter les zones completement noires
//   - Ecrire la couleur finale dans outColor
