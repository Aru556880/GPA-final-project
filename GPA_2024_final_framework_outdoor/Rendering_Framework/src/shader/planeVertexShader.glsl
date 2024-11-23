#version 430 core

layout(location=0) in vec3 v_vertex;
layout(location=1) in vec3 v_normal;
layout(location=2) in vec3 v_uv;

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projMat;
uniform vec3 planePosition;

// out 
out vec3 f_uv;


// render plane
void main() {
	//vec4 worldPosition = vec4(planePosition + v_vertex, 1.0);
	vec4 worldPosition = vec4(v_vertex + planePosition, 1.0) ; // + vec4(0, 30, 0, 0);

    gl_Position = projMat * viewMat * modelMat * worldPosition;

	f_uv = v_uv;

}