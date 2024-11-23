#version 430 core

in vec3 f_uv;
layout (binding = 4) uniform sampler2D albedoTexture;	// tex unit 4 for plane texture

out vec4 fragColor;

// render plane
void main() {
	// output color
	fragColor = texture(albedoTexture, f_uv.xy); //vec4(0, 0, 1, 1);
}