#version 430 core

                                                          
layout (location = 0) out vec4 frag_position;
layout (location = 1) out vec4 frag_normal;  
layout (location = 2) out vec4 frag_diffuse;  
                                      
in VS_OUT                                                 
{                                                         
    vec3 position;                                        
    vec3 normal;                                          
    vec2 texcoord;                                        
} fs_in;                                                  
                                                       
uniform sampler2D tex_diffuse;                            

void main(void)                                           
{                                                         

    frag_position = vec4(fs_in.position * 0.5 + 0.5, 1.0);            
	frag_normal = vec4(normalize(fs_in.normal) * 0.5 + 0.5, 1.0);     
    vec4 texel = texture(tex_diffuse, fs_in.texcoord) ;
	
	if(texel.a < 0.5)
        discard;
    else
	    frag_diffuse = vec4(texel.rgb, 1.0);  

}                                                         