#version 450
#extension GL_EXT_scalar_block_layout : require

layout(scalar, set = 0, binding = 0) readonly buffer Positions  { vec3 positions[]; };
layout(scalar, set = 0, binding = 1) readonly buffer Normals    { vec3 normals[]; };
layout(scalar, set = 0, binding = 2) readonly buffer TexCoords  { vec2 texcoords[]; };
layout(scalar, set = 0, binding = 3) readonly buffer Tangents   { vec3 tangents[]; };
layout(        set = 0, binding = 4) readonly buffer PosIndices { uint posIndices[]; };
layout(        set = 0, binding = 5) readonly buffer NrmIndices { uint nrmIndices[]; };
layout(        set = 0, binding = 6) readonly buffer TexIndices { uint texIndices[]; };

layout(set = 0, binding = 7) uniform UBO {
    mat4 viewProj;
    vec4 camPos;
} ubo;

layout(push_constant) uniform PC { mat4 model; } pc;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 fragTangent;
layout(location = 2) out vec2 fragUV;
layout(location = 3) out vec3 fragWorldPos;
layout(location = 4) out vec3 fragViewTan; // Direction vue en espace tangent

void main() {
    uint pi = posIndices[gl_VertexIndex];
    uint ni = nrmIndices[gl_VertexIndex];
    uint ti = texIndices[gl_VertexIndex];

    vec4 worldPos  = pc.model * vec4(positions[pi], 1.0);
    mat3 normalMat = transpose(inverse(mat3(pc.model)));

    vec3 N = normalize(normalMat * normals[ni]);
    vec3 T = normalize(mat3(pc.model) * tangents[ni]);

    fragWorldPos = worldPos.xyz;
    fragNormal   = N;
    fragTangent  = T;
    fragUV       = vec2(texcoords[ti].x, 1.0 - texcoords[ti].y);

    gl_Position  = ubo.viewProj * worldPos;

    // -----------------------------------------------------------------------
    // TODO : Calculer la direction de vue en espace tangent.
    //
    // 1. Re-orthogonaliser T par rapport a N (Gram-Schmidt) :
    //      T = normalize(T - dot(T, N) * N);
    //      vec3 B = cross(N, T);
    //
    // 2. Construire la matrice TBN inverse (TBN^T pour une base orthonormale) :
    //      mat3 TBN_inv = transpose(mat3(T, B, N));
    //
    // 3. Calculer la direction de vue en espace monde puis la transformer :
    //      vec3 viewDir = normalize(ubo.camPos.xyz - worldPos.xyz);
    //      fragViewTan  = TBN_inv * viewDir;
    // -----------------------------------------------------------------------
    fragViewTan = vec3(0.0, 0.0, 1.0); // Placeholder : a remplacer
}
