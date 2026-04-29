#version 450

layout(set = 0, binding = 9) uniform sampler2D normalMap;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec3 fragTangent;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 N = normalize(fragNormal);
    vec3 T = normalize(fragTangent);
    T = normalize(T - dot(T, N) * N); // Re-orthogonalisation de Gram-Schmidt
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);

    // Decodage de la normal map : [0,1] -> [-1,1]
    vec3 nSample = texture(normalMap, fragUV).rgb * 2.0 - 1.0;
    vec3 normal  = normalize(TBN * nSample);

    vec3 lightDir = normalize(vec3(1.0, 2.0, 1.0));
    float diffuse = max(dot(normal, lightDir), 0.0);
    float ambient = 0.15;

    outColor = vec4(diffuse + ambient);
}
