
#version 450
#extension GL_EXT_scalar_block_layout : require
// Output color to fragment shader
layout(location = 0) out vec3 fragColor;

// Storage buffer containing vertex position
layout(scalar, set = 0, binding = 0) readonly buffer VertexBuffer {
    vec2 positions[];
};

// Storage buffer containing vertex colors
layout(scalar, set = 0, binding = 1) readonly buffer ColorBuffer {
    vec3 colors[];
};


void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}
