#version 450

layout(set = 0, binding = 8)  uniform sampler2D diffuseTex;
layout(set = 0, binding = 9)  uniform sampler2D normalMap;
layout(set = 0, binding = 10) uniform sampler2D heightMap;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec3 fragTangent;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec3 fragWorldPos;
layout(location = 4) in vec3 fragViewTan;

layout(location = 0) out vec4 outColor;

const float heightScale = 0.04;

void main() {
    // -----------------------------------------------------------------------
    // TODO 1 : Calculer le décalage UV par parallaxe.
    //
    //   a) Lire la hauteur depuis la height map :
    //        float h = texture(heightMap, fragUV).r;
    //
    //   b) Convertir en offset signe (centré autour de 0) :
    //        float height = h * heightScale - heightScale * 0.5;
    //
    //   c) Projeter la vue sur le plan tangent et decaler les UVs :
    //        vec3  V  = normalize(fragViewTan);
    //        vec2  uvOffset = V.xy / V.z * height;
    //        vec2  uv = fragUV + uvOffset;
    //
    // Utiliser `uv` a la place de `fragUV` pour tous les echantillonnages suivants.
    // -----------------------------------------------------------------------
    vec2 uv = fragUV; // Placeholder : a remplacer par le calcul ci-dessus

    // -----------------------------------------------------------------------
    // TODO 2 : Appliquer le normal mapping aux UVs decales.
    //
    // Reprenez le code de nrm.frag :
    //   - Re-orthogonaliser T, construire TBN
    //   - Decoder la normal map a `uv`
    //   - Calculer eclairage diffus + ambient
    //   - Echantillonner diffuseTex a `uv`
    // -----------------------------------------------------------------------

    // Fallback (a remplacer)
    vec3 lightDir = normalize(vec3(1.0, 2.0, 1.0));
    float diffuse = max(dot(normalize(fragNormal), lightDir), 0.0);
    outColor = texture(diffuseTex, uv) * (diffuse + 0.15);
}
