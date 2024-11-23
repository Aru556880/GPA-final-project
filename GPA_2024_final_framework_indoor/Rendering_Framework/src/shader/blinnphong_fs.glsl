#version 430 core

in vec3 f_viewVertex ;

layout (location = 0) out vec4 fragColor ;

layout(binding = 0) uniform sampler2D albedoTexture ;
layout(binding = 1) uniform sampler2DShadow shadow_tex;
layout(binding = 2) uniform sampler2D normalMap ;

layout(location = 13) uniform vec3 ambient_albedo;
layout(location = 14) uniform vec3 diffuse_albedo;
layout(location = 15) uniform vec3 specular_albedo;
layout(location = 16) uniform float shininess;
layout(location = 20) uniform int useNormalMap;

/*
in VS_OUT
{
	vec4 shadow_coord;
	vec3 texcoord;
	vec3 N;
	vec3 L;
	vec3 V;
} fs_in;*/

in VS_OUT
{
	vec4 shadow_coord;
	vec3 texcoord;
	vec3 normal;
	vec3 lightDir;
	vec3 eyeDir;
	vec3 t;
} fs_in;

vec3 Ia = vec3(0.1, 0.1, 0.1);
vec3 Id = vec3(0.7, 0.7, 0.7);
vec3 Is = vec3(0.2, 0.2, 0.2);

vec3 textureColor(){
	vec4 texel = texture(albedoTexture, fs_in.texcoord.xy);
	
	if(texel.a < 0.5)
		discard;
	else
		return texel.rgb; 	
}

/*
void main(){	
	vec3 Ka = ambient_albedo;
	vec3 Kd = diffuse_albedo;
	vec3 Ks = specular_albedo;

	Kd = textureColor().rgb ;

	// calculate shading
	vec3 N = normalize(fs_in.N);
	vec3 L = normalize(fs_in.L);
	vec3 V = normalize(fs_in.V);
	vec3 H = normalize(L + V);

	vec3 ambient = Ia * Ka;
	vec3 diffuse = Id * max(dot(N, L), 0) * Kd;
	vec3 specular = Is * pow(max(dot(N, H), 0), shininess) * Ks;
	vec4 shadingColor = vec4( ambient * 0.1 + diffuse + specular, 1.0);

	if(useNormalMap < 0)
		fragColor = textureProj(shadow_tex, fs_in.shadow_coord) * shadingColor ;
	else 
		fragColor = textureProj(shadow_tex, fs_in.shadow_coord) * vec4(Kd,1.0) ;
}*/

void main(){
	vec3 Ka = ambient_albedo;
	vec3 Kd = diffuse_albedo;
	vec3 Ks = specular_albedo;

	vec3 V = normalize(fs_in.eyeDir);                                             
    vec3 L = normalize(fs_in.lightDir);  
	vec3 H = normalize(L + V);
    vec3 N ;

    if(useNormalMap > 0)
		N = normalize(texture(normalMap, fs_in.texcoord.xy).rgb * 2.0 - vec3(1.0));
    else
		N = vec3(0.0, 0.0, 1.0);      
	
	Kd = textureColor().rgb ;                                                
	
	vec3 ambient = Ia * Ka;
	vec3 diffuse = Id * max(dot(N, L), 0) * Kd;
	vec3 specular = Is * pow(max(dot(N, H), 0), shininess) * Ks;
	vec4 shadingColor = vec4( diffuse + specular, 1.0);
	
	fragColor = textureProj(shadow_tex, fs_in.shadow_coord) * shadingColor ;
	fragColor = vec4(fs_in.t, 1.0);
}