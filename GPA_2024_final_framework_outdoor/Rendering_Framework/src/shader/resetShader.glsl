#version 430 core

/////////////////////////////////////// reset shader
struct DrawCommand {
    uint count;
    uint instanceCount;
    uint firstIndex;
    uint baseVertex;
    uint baseInstance;
};

struct RawInstanceProperties {
    vec4 position;
    mat4 rotationMatrix;
    ivec4 indices;
};

struct InstanceProperties {
    ivec4 indices;
};

// SSBO
layout (std430, binding = 1) buffer DrawCommandsBlock {
    DrawCommand commands[];
};
layout (std430, binding = 2) buffer RawInstanceData {
    RawInstanceProperties rawInstanceData[];
};

layout (std430, binding = 3) buffer CurrValidInstanceData {
    InstanceProperties currValidInstanceData[];
};

layout (local_size_x = 1) in;  // Adjust work group size as needed

void main() {
    commands[0].instanceCount = 0;
    commands[1].instanceCount = 0;
    commands[2].instanceCount = 0;
    commands[3].instanceCount = 0;
    commands[4].instanceCount = 0;
}