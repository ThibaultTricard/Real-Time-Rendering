#version 450
#extension GL_EXT_scalar_block_layout : require

layout(scalar, set = 0, binding = 0) readonly buffer Positions  { vec3 positions[]; };
layout(scalar, set = 0, binding = 1) readonly buffer Normals    { vec3 normals[]; };
layout(scalar, set = 0, binding = 2) readonly buffer TexCoords  { vec2 texcoords[]; };
layout(        set = 0, binding = 3) readonly buffer PosIndices { uint posIndices[]; };
layout(        set = 0, binding = 4) readonly buffer NrmIndices { uint nrmIndices[]; };
layout(        set = 0, binding = 5) readonly buffer TexIndices { uint texIndices[]; };

layout(set = 0, binding = 6) uniform UBO { mat4 viewProj; } ubo;
layout(push_constant) uniform PC { mat4 model; } pc;

layout(set = 0, binding = 7) uniform sampler2D heightMap;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragUV;

const float DISPLACEMENT_SCALE = 0.1;

void main() {
    uint pi = posIndices[gl_VertexIndex];
    uint ni = nrmIndices[gl_VertexIndex];
    uint ti = texIndices[gl_VertexIndex];

    vec2 uv = vec2(texcoords[ti].x, 1.0 - texcoords[ti].y);

    // TODO : Lire la hauteur depuis heightMap aux coordonnees uv.
    //        Deplacer la position le long de la normale :
    //          float height = texture(heightMap, uv).r;
    //          vec3 pos = positions[pi] + normals[ni] * height * DISPLACEMENT_SCALE;

    vec3  pos     = positions[pi]; // remplacer par la position deplacee
    vec4  worldPos = pc.model * vec4(pos, 1.0);

    mat3 normalMat = transpose(inverse(mat3(pc.model)));
    fragNormal = normalize(normalMat * normals[ni]);
    fragUV     = uv;

    gl_Position = ubo.viewProj * worldPos;
}
