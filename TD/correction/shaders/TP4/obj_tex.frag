#version 450

layout(set = 0, binding = 7) uniform sampler2D diffuseTex;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 lightDir  = normalize(vec3(1.0, 2.0, 1.0));
    float diffuse  = max(dot(normalize(fragNormal), lightDir), 0.0);
    float ambient  = 0.15;
    vec4 texColor  = texture(diffuseTex, fragUV);
    outColor = texColor * (diffuse + ambient);
}
