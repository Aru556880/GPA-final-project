#version 430 core

out VS_OUT                                                                   
{                                                                        
	vec2 texcoord;                                                       
}vs_out;     

void main()                          
{                                    
    const vec4 verts[4] = vec4[4](   
        vec4(-1.0, -1.0, 0.0, 1.0),  
        vec4( 1.0, -1.0, 0.0, 1.0),  
        vec4(-1.0,  1.0, 0.0, 1.0),  
        vec4( 1.0,  1.0, 0.0, 1.0)); 
    
    const vec2 texcoord[4] = vec2[4](
        vec2(0.0, 0.0),  
        vec2( 1.0, 0.0),  
        vec2(0.0,  1.0),  
        vec2( 1.0,  1.0));

    gl_Position = verts[gl_VertexID];
    vs_out.texcoord = texcoord[gl_VertexID];
}                                    