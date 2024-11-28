#version 430 core

                                                          
layout (location = 0) out vec4 frag_position;
layout (location = 1) out vec4 frag_normal;  
layout (location = 2) out vec4 frag_ambient;  
layout (location = 3) out vec4 frag_diffuse;  
layout (location = 4) out vec4 frag_specular;  
layout (location = 5) out vec4 frag_tangent;  
layout (location = 6) out vec4 frag_normalTex;  
                       
layout(location = 13) uniform vec3 ambient_albedo;
layout(location = 14) uniform vec3 diffuse_albedo;
layout(location = 15) uniform vec3 specular_albedo;

layout(location = 20) uniform int useNormalMap;


in VS_OUT                                                 
{                                                         
    vec3 position;                                        
    vec3 normal;      
    vec3 tangent;
    vec2 texcoord;                                        
} fs_in;                                                  
                                                       
layout(binding = 0) uniform sampler2D tex_diffuse;    
layout(binding = 1) uniform sampler2D tex_normal;

// 1: default
// 0: scene model
// -1: area light rectangle
// -2: point light sphere
uniform float modelType;

void main(void)                                           
{                                                         

    frag_position = vec4(fs_in.position, modelType);            
	frag_normal = vec4(fs_in.normal, 1.0);   
    frag_ambient = vec4(ambient_albedo, 1.0);   
    vec4 texel = texture(tex_diffuse, fs_in.texcoord) ;

    if(useNormalMap != 0)
        frag_normalTex = texture(tex_normal, fs_in.texcoord) ;
	else
        frag_normalTex = vec4(0.0);

	if(texel.a < 0.5)
        discard;
    else
	    frag_diffuse = vec4(texel.rgb, 1.0);  

    frag_specular = vec4(specular_albedo, 1.0);   
    frag_tangent = vec4(fs_in.tangent, 1.0);
}                                                         