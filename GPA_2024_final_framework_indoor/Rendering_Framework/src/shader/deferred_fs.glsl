#version 430 core

layout (location = 0) out vec4 frag_color;
                                          
uniform sampler2D position_map;           
//uniform sampler2D normal_map;             
//uniform sampler2D diffuse_map;     

in VS_OUT                                                                   
{                                                                        
	vec2 texcoord;                                                       
}fs_in;      

void main(){
	//vec3 position = texelFetch(position_map, ivec2(gl_FragCoord.xy), 0).rgb;
	//vec3 normal = texelFetch(normal_map, ivec2(gl_FragCoord.xy), 0).rgb;    
	//vec3 diffuse = texelFetch(diffuse_map, ivec2(gl_FragCoord.xy), 0).rgb;  

	vec3 position = texture(position_map, fs_in.texcoord).rgb;
    frag_color = vec4(position,1.0);
}