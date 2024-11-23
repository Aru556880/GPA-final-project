#version 430 core

layout (location = 0) out vec4 frag_color;
                                          
layout (location = 0) uniform sampler2D position_map;           
layout (location = 1) uniform sampler2D normal_map;    
layout (location = 2) uniform sampler2D ambient_map;   
layout (location = 3) uniform sampler2D diffuse_map;     
layout (location = 4) uniform sampler2D specular_map;   

in VS_OUT                                                                   
{                                                                        
	vec2 texcoord;                                                       
}fs_in;      

void main(){
	vec3 position = texture(position_map, fs_in.texcoord).rgb;
	vec3 normal = texture(normal_map, fs_in.texcoord).rgb;  
	vec3 diffuse = texture(position_map, fs_in.texcoord).rgb;

	vec3 texel = texture(position_map, fs_in.texcoord).rgb;
    frag_color = vec4(texel,1.0);
}