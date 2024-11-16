#version 430 core

layout(location=0) in vec3 v_vertex;
layout(location=1) in vec3 v_normal ;
layout(location=2) in vec3 v_uv ;

out vec3 f_viewVertex ;
out vec3 f_uv ;

layout(location = 0) uniform mat4 modelMat ;
layout(location = 1) uniform mat4 viewMat ;
layout(location = 2) uniform mat4 projMat ;
layout(location = 5) uniform int vertexProcessIdx ;
layout(location = 6) uniform sampler2D normalMap ;
layout(location = 11) uniform vec3 lightPos;
out VS_OUT
{
	vec3 N;
	vec3 L;
	vec3 V;
} vs_out;


void commonProcess(){
	vec4 worldVertex = modelMat * vec4(v_vertex, 1.0) ;
	vec4 worldNormal = modelMat * vec4(v_normal, 0.0) ;

	mat4 mv_matrix = viewMat * modelMat;
	vec4 viewVertex = viewMat * worldVertex ;
	vec4 viewNormal = viewMat * worldNormal ;
	
	f_viewVertex = viewVertex.xyz;
	f_uv = v_uv ;

	vec4 light_pos_view = viewMat * vec4(lightPos, 1.0);
	vs_out.N = mat3(mv_matrix) * v_normal;
	vs_out.L = light_pos_view.xyz - viewVertex.xyz;
	vs_out.V = - viewVertex.xyz;

	gl_Position = projMat * viewVertex ;
}


void main(){
	commonProcess() ;
}