#version 430 core

layout (location = 0) out vec4 frag_color;
                                          
layout (binding = 0) uniform sampler2D scene;
layout (binding = 1) uniform sampler2D bloomBlur;

in VS_OUT                                                                   
{                                                                        
	vec2 texcoord;                                                       
}fs_in;      

float exposure = 0.3f;

void main(){

    const float gamma = 2.2;
    vec3 hdrColor = texture(scene, fs_in.texcoord).rgb;
    vec3 bloomColor = texture(bloomBlur, fs_in.texcoord).rgb;
    hdrColor += bloomColor; // additive blending
    // tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    // also gamma correct while we're at it       
    result = pow(result, vec3(1.0 / gamma));

    frag_color = vec4(result, 1.0);
}