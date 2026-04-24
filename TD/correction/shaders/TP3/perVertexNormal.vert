#version 450
#extension GL_EXT_scalar_block_layout : require

layout(location = 0) out vec3 fragNormal;

layout(scalar, set = 0, binding = 0) readonly buffer Positions   { vec3 positions[];    };
layout(scalar, set = 0, binding = 1) readonly buffer Normals     { vec3 normals[];      };
layout(        set = 0, binding = 2) readonly buffer PosIndices  { uint posIndices[];   };

layout(set = 0, binding = 3) uniform MVP { mat4 mvp; };

void main() {
    uint posIdx = posIndices[gl_VertexIndex];
    uint nrmIdx = posIndices[gl_VertexIndex];
    gl_Position = mvp * vec4(positions[posIdx], 1.0);
    fragNormal  = normals[nrmIdx];
}
