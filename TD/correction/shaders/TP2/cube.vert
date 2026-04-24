#version 450
#extension GL_EXT_scalar_block_layout : require

layout(location = 0) out vec3 fragColor;

layout(scalar, set = 0, binding = 0) readonly buffer Positions { vec3 positions[]; };
layout(scalar, set = 0, binding = 1) readonly buffer Colors    { vec3 colors[];    };
layout(        set = 0, binding = 2) readonly buffer Indices   { uint indices[];   };

layout(set = 0, binding = 3) uniform MVP {
    mat4 mvp;
};

void main() {
    uint index  = indices[gl_VertexIndex];
    gl_Position = mvp * vec4(positions[index], 1.0);
    fragColor   = colors[index];
}
