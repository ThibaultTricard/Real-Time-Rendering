#version 450

layout(set = 0, binding = 7) uniform sampler2D diffuseTex;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 lightDir = normalize(vec3(1.0, 2.0, 1.0));
    float diffuse = max(dot(normalize(fragNormal), lightDir), 0.0);
    float ambient = 0.15;

    // textureLod(..., 0) forces mip level 0 — shows moiré / aliasing
    // texture() selects the right mip level automatically — no aliasing
    vec4 texColor = texture(diffuseTex, fragUV);
    //vec4 texColor = textureLod(diffuseTex, fragUV, 0.0);

    outColor = texColor * (diffuse + ambient);
}
