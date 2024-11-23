#version 430 core                       
                                          
layout (location = 0) in vec3 position;   
layout (location = 1) in vec3 normal;     
layout (location = 2) in vec2 texcoord;	 
layout (location = 3) in vec2 tangent;	 
	                                      
out VS_OUT	                              
{	                                      
    vec3 position;                        
    vec3 normal;	                      
    vec2 texcoord;	                      
} vs_out;	                              
	                                      

layout(location = 0) uniform mat4 modelMat ;
layout(location = 1) uniform mat4 viewMat ;
layout(location = 2) uniform mat4 projMat ;


void main(){	
	vec4 P = vec4(position, 1.0); 
		
	mat4 mv_matrix = viewMat * modelMat;	        
	gl_Position = projMat * mv_matrix * P;	                    
	vs_out.position = (modelMat* P).xyz;	            
	vs_out.normal = normalize(mat3(modelMat) * normal);	
	vs_out.texcoord = texcoord;	  
}