#version 450

layout(set = 0, binding = 7) uniform sampler2D diffuseTex;
layout(set = 0, binding = 8) uniform sampler2D normalMap;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

void main() {
    // Compute TBN from screen-space derivatives
    vec3 N  = normalize(fragNormal);
    vec3 dp1 = dFdx(fragWorldPos);
    vec3 dp2 = dFdy(fragWorldPos);
    vec2 du1 = dFdx(fragUV);
    vec2 du2 = dFdy(fragUV);

    float det = du1.x * du2.y - du1.y * du2.x;
    vec3 T = normalize((dp1 * du2.y - dp2 * du1.y) / det);
    vec3 B = normalize((dp2 * du1.x - dp1 * du2.x) / det);
    mat3 TBN = mat3(T, B, N);

    // Sample and decode normal map [0,1] -> [-1,1]
    vec3 nMapSample = texture(normalMap, fragUV).rgb * 2.0 - 1.0;
    vec3 normal = normalize(TBN * nMapSample);

    vec3 lightDir = normalize(vec3(1.0, 2.0, 1.0));
    float diffuse = max(dot(normal, lightDir), 0.0);
    float ambient = 0.15;

    vec4 texColor = textureGrad(diffuseTex, fragUV, dFdx(fragUV), dFdy(fragUV));
    outColor = texColor * (diffuse + ambient);
}
