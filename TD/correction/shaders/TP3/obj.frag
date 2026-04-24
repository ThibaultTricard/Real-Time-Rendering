#version 450

layout(location = 0) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 lightDir = normalize(vec3(1.0, 2.0, 1.0));
    float diffuse = max(dot(normalize(fragNormal), lightDir), 0.0);
    float ambiant = 0.15;
    vec3 albedo = vec3(0.8, 0.75, 0.7);
    vec3 color = albedo * (diffuse) + ambiant;
    outColor = vec4(color, 1.0);
}
