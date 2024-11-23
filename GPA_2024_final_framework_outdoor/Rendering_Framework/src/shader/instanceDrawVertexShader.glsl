#version 430 core

layout(location=0) in vec3 v_vertex;
layout(location=1) in vec3 v_normal;
layout(location=2) in vec3 v_uv;

uniform mat4 um4mv;
uniform mat4 um4p;

out vec3 f_uv;

///////////////////////////////////////
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

uint grassB_base = 567237;
uint bush01_base = 1010;
uint bush05_base = 2797;
uint building1_base = 299;
uint building2_base = 298;

void main() {
    uint instanceID;
    if (v_uv.z == 0) {                  // uv.z is texture ID
        instanceID = gl_InstanceID;
    } else if (v_uv.z == 1) {
        instanceID = gl_InstanceID + grassB_base;
    } else if (v_uv.z == 2) {
        instanceID = gl_InstanceID + grassB_base + bush01_base;
    } else if (v_uv.z == 3) {
        instanceID = gl_InstanceID + grassB_base + bush01_base + bush05_base;
    } else if (v_uv.z == 4) {
        instanceID = gl_InstanceID + grassB_base + bush01_base + bush05_base + building1_base;
    }
    
    
    uint instanceIndex = currValidInstanceData[instanceID].indices.x;
    vec4 instancePosition = rawInstanceData[instanceIndex].position;
    mat4 instanceRotationMatrix = rawInstanceData[instanceIndex].rotationMatrix;

    // compute transformed position
    vec4 worldPosition;
    /*
    if (v_uv.z == 1) {
        worldPosition = instanceRotationMatrix * vec4(v_vertex, 1.0);
    } else {
        worldPosition = instancePosition + instanceRotationMatrix * vec4(v_vertex, 1.0);
    }
    */
    worldPosition = instancePosition + instanceRotationMatrix * vec4(v_vertex, 1.0);

    vec4 pos = um4mv * worldPosition;
    gl_Position = um4p * pos;

    f_uv = v_uv;

    //gl_Position = vec4(v_vertex, 1.0);
}