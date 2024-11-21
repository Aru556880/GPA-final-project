#version 430 core
                       
layout (location = 0) uniform mat4 mvp;                      
                                       
layout (location = 0) in vec3 position;
layout(location=1) in vec3 v_normal ;
layout(location=2) in vec3 v_uv ; 


void main(void)                        
{                     

    gl_Position =mvp * vec4(position, 1.0);      
}                                               