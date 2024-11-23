#version 430 core

in vec3 f_uv;

out vec4 fragColor;

uniform sampler2DArray albedoTextureArray;
uniform mat4 um4mv;
uniform mat4 um4p;

void main() {
	//vec3 debugUV = vec3(0.5, 0.5, 3); // Middle of the first layer
	//vec4 texel = texture(albedoTextureArray, debugUV);
	vec4 texel = texture(albedoTextureArray, f_uv);
	// discard the transparent texel
	if (texel.a < 0.5) {
		discard;
	}
	

	// output color
	//fragColor = vec4(color, texel.a);
	fragColor = vec4(texel);
	//fragColor = vec4(vec3(1, 0, 0), texel.a);
}