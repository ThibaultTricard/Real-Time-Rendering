#version 450
#extension GL_EXT_scalar_block_layout : require

layout(scalar, set = 0, binding = 0) readonly buffer Positions     { vec3 positions[]; };
layout(scalar, set = 0, binding = 1) readonly buffer Normals       { vec3 normals[]; };
layout(scalar, set = 0, binding = 2) readonly buffer PosIndices    { uint posIndices[]; };
layout(scalar, set = 0, binding = 3) readonly buffer NormalIndices { uint normalIndices[]; };
layout(set = 0, binding = 4) uniform UBO { mat4 mvp; } ubo;

layout(location = 0) out vec3 fragNormal;

void main() {
    uint posIdx = posIndices[gl_VertexIndex];
    uint nrmIdx = normalIndices[gl_VertexIndex];

    gl_Position = ubo.mvp * vec4(positions[posIdx], 1.0);
    fragNormal  = normalize(normals[nrmIdx]);
}
