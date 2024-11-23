#version 430 core

layout(location=0) in vec3 v_vertex;
layout(location=1) in vec3 v_normal ;
layout(location=2) in vec3 v_uv ;
layout(location=3) in vec3 v_tangent ;

out vec3 f_viewVertex ;
out vec3 f_uv ;

layout(location = 0) uniform mat4 modelMat ;
layout(location = 1) uniform mat4 viewMat ;
layout(location = 2) uniform mat4 projMat ;
layout(location = 3) uniform mat4 shadow_matrix;

layout(location = 11) uniform vec3 lightPos;
layout(location = 20) uniform int useNormalMap;

/*
out VS_OUT
{
	vec4 shadow_coord;
	vec3 texcoord;
	vec3 N;
	vec3 L;
	vec3 V;
} vs_out;*/

out VS_OUT
{
	vec4 shadow_coord;
	vec3 texcoord;
	vec3 normal;
	vec3 lightDir;
	vec3 eyeDir;
	vec3 t;
} vs_out;

/*
void main(){
	vec4 position = vec4(v_vertex, 1.0) ;

	vec4 worldVertex = modelMat * position;
	vec4 worldNormal = modelMat * vec4(v_normal, 0.0) ;

	mat4 mv_matrix = viewMat * modelMat;
	vec4 viewVertex = viewMat * worldVertex ;
	vec4 viewNormal = viewMat * worldNormal ;
	
	f_viewVertex = viewVertex.xyz;
	vs_out.texcoord = v_uv ;

	vec4 light_pos_view = viewMat * vec4(lightPos, 1.0);
	vs_out.N = mat3(mv_matrix) * v_normal;
	vs_out.L = light_pos_view.xyz - viewVertex.xyz;
	vs_out.V = - viewVertex.xyz;

	vs_out.shadow_coord = shadow_matrix * position;

	gl_Position = projMat * viewVertex ;
}*/

void main(){
	vec4 position = vec4(v_vertex, 1.0);
	mat4 mv_matrix = viewMat * modelMat;

	vec4 viewVertex = mv_matrix * position ;
	vec4 light_pos_view = viewMat * vec4(lightPos, 1.0);
	
	vec3 V = viewVertex.xyz;                               
    vec3 N = normalize(mat3(mv_matrix) * v_normal); 
    vec3 T = normalize(mat3(mv_matrix) * v_tangent);
	vec3 B = cross(N, T);
	vec3 L = light_pos_view.xyz - viewVertex.xyz;
	
	vs_out.lightDir = normalize(vec3(dot(L, T), dot(L, B), dot(L, N)));
	V = -viewVertex.xyz;  

	vs_out.t = (viewMat * vec4(1.0, 0.0, 0.0, 0.0)).xyz;

    vs_out.eyeDir = normalize(vec3(dot(V, T), dot(V, B), dot(V, N)));     
    vs_out.texcoord = v_uv;                                                                                                                                               
    vs_out.normal = N;                                                    
    vs_out.shadow_coord = shadow_matrix * position;  
	
    gl_Position = projMat * viewVertex;                                        
}