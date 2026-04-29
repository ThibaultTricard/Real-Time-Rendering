#version 450
#extension GL_EXT_scalar_block_layout : require

layout(scalar, set = 0, binding = 0) readonly buffer Positions  { vec3 positions[]; };
layout(scalar, set = 0, binding = 1) readonly buffer Normals    { vec3 normals[]; };
layout(scalar, set = 0, binding = 2) readonly buffer TexCoords  { vec2 texcoords[]; };
layout(        set = 0, binding = 3) readonly buffer PosIndices { uint posIndices[]; };
layout(        set = 0, binding = 4) readonly buffer NrmIndices { uint nrmIndices[]; };
layout(        set = 0, binding = 5) readonly buffer TexIndices { uint texIndices[]; };

layout(set = 0, binding = 6) uniform ViewProj { mat4 viewProj; } ubo;
layout(push_constant) uniform PushConstants { mat4 model; } pc;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out vec3 fragWorldPos;

void main() {
    uint posIdx = posIndices[gl_VertexIndex];
    uint nrmIdx = nrmIndices[gl_VertexIndex];
    uint texIdx = texIndices[gl_VertexIndex];

    vec3 pos    = positions[posIdx];
    vec3 normal = normals[nrmIdx];
    vec2 uv     = texcoords[texIdx];

    vec4 worldPos = pc.model * vec4(pos, 1.0);
    fragWorldPos  = worldPos.xyz;
    fragNormal    = mat3(pc.model) * normal; // valid for pure rotations
    fragUV        = vec2(uv.x, 1.0 - uv.y);

    gl_Position = ubo.viewProj * worldPos;
}
