#version 450

layout(set = 0, binding = 8) uniform sampler2D diffuseTex;
layout(set = 0, binding = 9) uniform sampler2D normalMap;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec3 fragTangent;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

void main() {
    // -----------------------------------------------------------------------
    // TODO : Implémenter le normal mapping.
    //
    // 1. Re-orthogonaliser la tangente par rapport a la normale (Gram-Schmidt) :
    //      vec3 N = normalize(fragNormal);
    //      vec3 T = normalize(fragTangent - dot(fragTangent, N) * N);
    //      vec3 B = cross(N, T);
    //      mat3 TBN = mat3(T, B, N);
    //
    // 2. Lire la normal map et decoder [0,1] -> [-1,1] :
    //      vec3 nSample = texture(normalMap, fragUV).rgb * 2.0 - 1.0;
    //
    // 3. Transformer le vecteur tangent en espace monde :
    //      vec3 normal = normalize(TBN * nSample);
    //
    // 4. Calculer l'eclairage diffus + ambient avec ce vecteur :
    //      vec3 lightDir = normalize(vec3(1.0, 2.0, 1.0));
    //      float diffuse = max(dot(normal, lightDir), 0.0);
    //      float ambient = 0.15;
    //      outColor = texture(diffuseTex, fragUV) * (diffuse + ambient);
    // -----------------------------------------------------------------------

    // Fallback (a remplacer par votre implementation)
    vec3 lightDir = normalize(vec3(1.0, 2.0, 1.0));
    float diffuse = max(dot(normalize(fragNormal), lightDir), 0.0);
    outColor = texture(diffuseTex, fragUV) * (diffuse + 0.15);
}
