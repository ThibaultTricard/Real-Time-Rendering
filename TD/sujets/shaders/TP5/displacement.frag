#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

void main() {
    vec3  lightDir = normalize(vec3(1.0, 2.0, 1.0));
    float diffuse  = max(dot(normalize(fragNormal), lightDir), 0.0);
    outColor = vec4(vec3(diffuse + 0.15), 1.0);
}
