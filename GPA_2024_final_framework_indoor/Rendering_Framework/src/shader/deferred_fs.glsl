#version 430 core

layout (location = 0) out vec4 frag_color;
                                          
layout (binding = 0) uniform sampler2D position_map;   // in world space   
layout (binding = 1) uniform sampler2D normal_map;     // in world space, gbuffer normal map
layout (binding = 2) uniform sampler2D ambient_map;   
layout (binding = 3) uniform sampler2D diffuse_map;     
layout (binding = 4) uniform sampler2D specular_map;
layout (binding = 5) uniform sampler2D tangent_map;    // tangent is in view space

layout(binding = 6) uniform sampler2D normalTexture_map; // normal mapping texture
layout (binding = 7) uniform sampler2DShadow dirLightShadow_map;
layout (binding = 8) uniform samplerCube  pointLightShadow_map;



layout(location = 1) uniform mat4 viewMat ;
layout(location = 3) uniform mat4 shadow_matrix;
layout(location = 11) uniform vec3 light_pos_world;
layout(location = 18) uniform float far_plane;
layout(location = 21) uniform int deferred_map_type;

in VS_OUT                                                                   
{                                                                        
	vec2 texcoord;                                                       
}fs_in;      

vec3 Ia = vec3(0.1, 0.1, 0.1);
vec3 Id = vec3(0.7, 0.7, 0.7);
vec3 Is = vec3(0.2, 0.2, 0.2);

float shininess = 225.0;

float PointLightShadowCalculation(vec3 fragPos);

void main(){
	vec3 world_position = texture(position_map, fs_in.texcoord).rgb;
	vec3 world_normal = texture(normal_map, fs_in.texcoord).rgb;  
    vec3 world_tangent = texture(tangent_map, fs_in.texcoord).rgb; // normalized mv tangent
    vec3 normal_tex = texture(normalTexture_map, fs_in.texcoord).rgb;  

	vec3 Ka = texture(ambient_map, fs_in.texcoord).rgb;
	vec3 Kd = texture(diffuse_map, fs_in.texcoord).rgb;
	vec3 Ks = texture(specular_map, fs_in.texcoord).rgb;

	vec4 viewVertex = viewMat * vec4(world_position, 1.0) ;

    // calculate blinnphong shading color 
    vec4 light_pos_view = viewMat * vec4(light_pos_world, 1.0);
	vec3 N = normalize(mat3(viewMat) * world_normal);

    // This tangent is a bug, cannot figure out what's wrong
    vec3 T = world_tangent;
    

	vec3 L = light_pos_view.xyz - viewVertex.xyz;
    float distance = length(L);
    L = normalize(L);
    vec3 B = cross(N, T);
    vec3 V = normalize(- viewVertex.xyz);
    vec3 H = normalize(L + V);
    
    // For normal mapping
    vec3 L_t = normalize(vec3(dot(L, T), dot(L, B), dot(L, N)));
    vec3 V_t = normalize(vec3(dot(V, T), dot(V, B), dot(V, N)));
    vec3 H_t = normalize(L_t+ V_t);
	vec3 N_t = vec3(0.0, 0.0, 1.0);  
    

    vec3 ambient = Ia * Ka;
	vec3 diffuse = Id * max(dot(N, L), 0) * Kd;
	vec3 specular = Is * pow(max(dot(N, H), 0), shininess) * Ks;

    if(length(normal_tex) > 0){
        N_t = normalize(normal_tex * 2.0 - vec3(1.0));
	    diffuse = Id * max(dot(N_t, L_t), 0) * Kd;
	    specular = Is * pow(max(dot(N_t, H_t), 0), shininess) * Ks;
    }
    else{
        vec3 diffuse = Id * max(dot(N, L), 0) * Kd;
	    vec3 specular = Is * pow(max(dot(N, H), 0), shininess) * Ks;
    }

	vec3 shadingColor = ambient*0.1 + diffuse + specular;

    // point light color:
    float c1 = 1.0;
    float c2 = 0.7;
    float c3 = 0.14;

    float f_a = min( 1/(c1 + c2 * distance + c3 * distance * distance), 1.0 );
    vec3 pointLight_color = ambient*0.1 + (diffuse + specular) * f_a;

    vec4 shadow_coord = shadow_matrix * vec4(world_position, 1.0);
    vec3 shadow_color = textureProj(dirLightShadow_map, shadow_coord) * shadingColor;

	vec3 color;
	switch (deferred_map_type) {
        case 0:
            color = normalize(world_position) * 0.5 + 0.5;
            break;
        case 1:
            color = normalize(world_normal) * 0.5 + 0.5;
            break;
        case 2:
            color = Ka;
            break;
        case 3:
            color = Kd;
            break;
        case 4:
            color = Ks;
            break;
        case 5:
            color = shadingColor;
            break;
        case 6: 
            color = shadow_color; 
            break;
        case 7:
            float shadow = PointLightShadowCalculation(world_position);    
            //color =  vec3(shadow); //DEBUG;
            color = (1.0-shadow) * pointLight_color ;
            break;
        default:
            color = vec3(1.0);
            break;
    }


    frag_color = vec4(color,1.0);
}

float PointLightShadowCalculation(vec3 fragPos)
{
    // Get vector between fragment position and light position
    vec3 fragToLight = fragPos - light_pos_world;

    // Use the light to fragment vector to sample from the depth map    
    float closestDepth = texture(pointLightShadow_map, fragToLight).r;
    //return closestDepth ;
    
    // It is currently in linear range between [0,1]. Re-transform back to original value
    closestDepth *= 10.0;

    // Now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    //return currentDepth / 10.0;
    // Now test for shadows
    float bias = 0.05; 
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}

