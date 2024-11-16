#version 430 core

in vec3 f_viewVertex ;
in vec3 f_uv ;

layout (location = 0) out vec4 fragColor ;

layout(location = 3) uniform int pixelProcessId;
layout(location = 4) uniform sampler2D albedoTexture ;
layout(location = 12) uniform int useTexture; // useTextrue < 0: does not have texture

layout(location = 13) uniform vec3 ambient_albedo;
layout(location = 14) uniform vec3 diffuse_albedo;
layout(location = 15) uniform vec3 specular_albedo;
layout(location = 16) uniform float shininess;


in VS_OUT
{
	vec3 N;
	vec3 L;
	vec3 V;
} fs_in;

vec3 Ia = vec3(0.1, 0.1, 0.1);
vec3 Id = vec3(0.7, 0.7, 0.7);
vec3 Is = vec3(0.2, 0.2, 0.2);

vec3 textureColor(){
	vec4 texel = texture(albedoTexture, f_uv.xy) ;
	
	if(texel.a < 0.5)
		discard;
	else
		return texel.rgb; 	
}

void ShadingColor(){
	
}

void main(){	
	vec3 Ka = ambient_albedo;
	vec3 Kd = diffuse_albedo;
	vec3 Ks = specular_albedo;

	if(useTexture > 0)
		Kd = textureColor().rgb ;

	// calculate shading
	vec3 N = normalize(fs_in.N);
	vec3 L = normalize(fs_in.L);
	vec3 V = normalize(fs_in.V);
	vec3 H = normalize(L + V);

	vec3 ambient = Ia * Ka;
	vec3 diffuse = Id * max(dot(N, L), 0) * Kd;
	vec3 specular = Is * pow(max(dot(N, H), 0), shininess) * Ks;
	
	fragColor = vec4( diffuse * 0.8 + specular, 1.0);
}