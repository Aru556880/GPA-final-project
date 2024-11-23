#version 430 core

layout(location=0) in vec3 v_vertex;
layout(location=1) in vec3 v_normal;
layout(location=2) in vec3 v_uv;
layout(location=3) in vec3 v_tangent;
layout(location=4) in vec3 v_bitangent;

//uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projMat;
uniform vec3 stonePosition;

// out 
out vec3 f_uv;

out vec3 f_tangent;
out vec3 f_bitangent;

// render plane
void main() {
	// no need  to update model matrix
	mat4 modelMat = mat4(1.0f);

	// put stone in world space
	vec4 worldPosition = vec4(v_vertex + vec3(25.92, 18.27, 11.75), 1.0) ;

    gl_Position = projMat * viewMat * modelMat * worldPosition;

	f_uv = v_uv;

	//
	f_tangent = v_tangent;
	f_bitangent = v_bitangent;
}