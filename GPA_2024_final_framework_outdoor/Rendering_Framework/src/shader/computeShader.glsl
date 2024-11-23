#version 430 core

/////////////////////////////////////// cimpute shader
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

layout (local_size_x = 1024) in;  // Adjust work group size as needed

uniform mat4 um4mv;
uniform mat4 um4p;

uint grassB_UNIQUE_IDX = 0;
uint bush01_UNIQUE_IDX = 0;
uint bush05_UNIQUE_IDX = 0;
uint building1_UNIQUE_IDX = 0;
uint building2_UNIQUE_IDX = 0;

uint grassB_base = 567237;
uint bush01_base = 1010;
uint bush05_base = 2797;
uint building1_base = 299;
uint building2_base = 298;

void main() {
    uint id = gl_GlobalInvocationID.x;
    
    // for View Culling 
    mat4 vp = um4p * um4mv;
    int near_clip = 0;
    int far_clip = 400;

    // Object's world position
    vec3 objectPosition = rawInstanceData[id].position.xyz;

    // Bounding sphere parameters
    vec3 sphereCenter;
    float sphereRadius;

    // get the culling sphere eadius for different mesh
    if (id < grassB_base) {
            sphereCenter = vec3(0.0, 0.66, 0.0);
            sphereRadius = 1.4;
        }
        else if (id < grassB_base + bush01_base) {
            sphereCenter = vec3(0.0, 2.55, 0.0);
            sphereRadius = 3.4;
        }
        else if (id < grassB_base + bush01_base + bush05_base) {
            sphereCenter = vec3(0.0, 1.76, 0.0);
            sphereRadius = 2.6;
        }
        else if (id < grassB_base + bush01_base + bush05_base + building1_base) {
            sphereCenter = vec3(0.0, 10.2, 0.0);
            sphereRadius = 8.5;
        }
        else if (id < grassB_base + bush01_base + bush05_base + building1_base + building2_base) {
            sphereCenter = vec3(0.0, 8.5, 0.0);
            sphereRadius = 10.2;
        }
    
    // Transform the sphere center relative to the object's position
    vec4 worldSphereCenter = vec4(objectPosition + sphereCenter, 1.0);

    // Transform sphere center into clip space
    vec4 viewSphereCenter = um4mv * worldSphereCenter;
    vec4 clipSphereCenter = vp * worldSphereCenter;
    float clipSpaceW = clipSphereCenter.w; // Depth in view space

    // Compute clip-space radius
    float clipRadius = sphereRadius * abs(um4p[0][0]); // Assumes uniform scaling

    
    if (clipSphereCenter.x > clipSphereCenter.w + clipRadius ||     // bounding sphere cullling
        clipSphereCenter.x < -clipSphereCenter.w - clipRadius ||
        clipSphereCenter.y > clipSphereCenter.w + clipRadius || 
        clipSphereCenter.y < -clipSphereCenter.w - clipRadius ||
        clipSphereCenter.z > clipSphereCenter.w + clipRadius || 
        clipSphereCenter.z < 0.0 - clipRadius ||
        (clipSpaceW < near_clip - sphereRadius) ||
        (clipSpaceW > far_clip + sphereRadius)) {
            // Object is outside the view frustum
            return;
    } else {                        // draw instance indeces stored in currValidInstanceData
        if (id < grassB_base) {
            grassB_UNIQUE_IDX = atomicAdd(commands[0].instanceCount, 1);
            currValidInstanceData[grassB_UNIQUE_IDX].indices = rawInstanceData[id].indices;
        }
        else if (id < grassB_base + bush01_base) {
            bush01_UNIQUE_IDX = atomicAdd(commands[1].instanceCount, 1);
            currValidInstanceData[bush01_UNIQUE_IDX + grassB_base].indices = rawInstanceData[id].indices;
        }
        else if (id < grassB_base + bush01_base + bush05_base) {
            bush05_UNIQUE_IDX = atomicAdd(commands[2].instanceCount, 1);
            currValidInstanceData[bush05_UNIQUE_IDX + grassB_base + bush01_base].indices = rawInstanceData[id].indices;
        }
        else if (id < grassB_base + bush01_base + bush05_base + building1_base) {
            building1_UNIQUE_IDX = atomicAdd(commands[3].instanceCount, 1);
            currValidInstanceData[building1_UNIQUE_IDX + grassB_base + bush01_base + bush05_base].indices = rawInstanceData[id].indices;
        }
        
        else if (id < grassB_base + bush01_base + bush05_base + building1_base + building2_base) {
            building2_UNIQUE_IDX = atomicAdd(commands[4].instanceCount, 1);
            currValidInstanceData[building2_UNIQUE_IDX + grassB_base + bush01_base + bush05_base + building1_base].indices = rawInstanceData[id].indices;
        }  
    }



}
