#version 430 core

in vec3 f_uv;
in vec3 f_tangent;
in vec3 f_bitangent;

layout (binding = 5) uniform sampler2D albedoTexture;	// tex unit 5 for stone base color texture

out vec4 fragColor;

// render stone
void main() {
	
	// debugging tangent
	/*
	vec3 tangentColor = normalize(f_tangent) * 0.5 + 0.5;
	vec3 bitangentColor = normalize(f_bitangent) * 0.5 + 0.5;
	fragColor = vec4(tangentColor, 1.0);
	fragColor = vec4(bitangentColor, 1.0);
	*/


	// output color
	fragColor = texture(albedoTexture, f_uv.xy); //vec4(0, 0, 1, 1);
}