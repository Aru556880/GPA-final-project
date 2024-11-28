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
layout(location = 18) uniform float far_plane;
layout(location = 21) uniform int deferred_map_type;

// features enable/disable
uniform int enableNormalMap;
uniform int enablePhongLight;
uniform int enableDirLightShadow;
uniform int enablePointLight;
uniform int enablePointLightShadow;
uniform int enableAreaLight;


// light in world space
uniform vec3 blinnPhongLightPos;
uniform vec3 pointLightPos;
uniform vec3 areaLightPos;

in VS_OUT                                                                   
{                                                                        
	vec2 texcoord;                                                       
}fs_in;      

vec3 Ia = vec3(0.01, 0.01, 0.01);
vec3 Id = vec3(0.7, 0.7, 0.7);
vec3 Is = vec3(0.2, 0.2, 0.2);

vec3 Ka;
vec3 Ks;
vec3 Kd;

float shininess = 225.0;
float PI = 3.14159265359;

vec3 BlinnPhongLightColor(vec3 worldLightPos, vec3 worldNormal, vec3 viewFragPosition, bool useNormalMap);
float DirLightShadow(vec3 fragPos);
float PointLightShadow(vec3 fragPos);
vec3 AreaLightColor(vec4 view_position, vec3 normal, vec3 viewDir, vec3 diffuseFactor);
vec3 CalculatePlaneIntersection(vec3 viewPosition, vec3 reflectionVector, vec3 lightDirection, vec3 rectangleLightCenter);

void main(){
    // see comment in geometry_fs for modelType id
    float modelType = texture(position_map, fs_in.texcoord).a;
    bool useNormalMap = texture(normalTexture_map, fs_in.texcoord).a > 0;
    vec3 final_color = vec3(0);

    if(modelType == -1.0){
        frag_color = vec4(0.8, 0.6, 0.0, 1.0);
        return;
    }
    else if(modelType == 1.0){
        frag_color = vec4(0.8,0.8,0.8,1.0);
    }

	vec3 world_position = texture(position_map, fs_in.texcoord).rgb;
	vec3 world_normal = texture(normal_map, fs_in.texcoord).rgb;  
    vec3 world_tangent = texture(tangent_map, fs_in.texcoord).rgb; // normalized mv tangent
    vec3 normal_tex = texture(normalTexture_map, fs_in.texcoord).rgb;  

	Ka = texture(ambient_map, fs_in.texcoord).rgb;
	Kd = texture(diffuse_map, fs_in.texcoord).rgb;
	Ks = texture(specular_map, fs_in.texcoord).rgb;

	vec4 view_position = viewMat * vec4(world_position, 1.0) ;
    vec4 view_normal = vec4( mat3(viewMat) * world_normal, 1.0) ;

    // 1: blinnphong lighting & shadow
	vec3 blinnPhongColor 
        = BlinnPhongLightColor(blinnPhongLightPos, view_normal.xyz, view_position.xyz, useNormalMap);
    float dirLightShadow = DirLightShadow(world_position);

    // 2: point light & shadow
    float c1 = 1.0;
    float c2 = 0.7;
    float c3 = 0.14;

    vec4 pointLightPos_view = viewMat * vec4(pointLightPos, 1.0);
    float distance = length(pointLightPos_view.xyz - view_position.xyz);

    float f_a = min( 1/(c1 + c2 * distance + c3 * distance * distance), 1.0 );
    vec3 pointLightColor = 
        BlinnPhongLightColor(pointLightPos, view_normal.xyz, view_position.xyz, useNormalMap) ;
    float pointLightShadow = PointLightShadow(world_position);

    // 3: area ligt
    vec3 areaLightColor = AreaLightColor(view_position, view_normal.xyz, - view_position.xyz, Kd);

	vec3 color = vec3(0);
    float shadow = 0;
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
            color = blinnPhongColor;
            break;
        case 6: 
            shadow = DirLightShadow(world_position);
            color = (1.0-shadow) * blinnPhongColor; 
            break;
        case 7:
            shadow = PointLightShadow(world_position);    
            //color =  vec3(shadow); //DEBUG;
            color = (1.0-shadow) * pointLightColor;
            break;
        case 8:
            color = AreaLightColor(view_position, view_normal.xyz, - view_position.xyz, Kd) ;
            break;
        default:
            color = vec3(1.0);
            break;
    }


    final_color = enablePhongLight * blinnPhongColor * (1.0 - dirLightShadow * enableDirLightShadow)+ 
                  enablePointLight * pointLightColor * (1.0 - pointLightShadow * enablePointLightShadow)+ 
                  enableAreaLight * areaLightColor;

    frag_color = vec4(final_color, 1.0);
}

vec3 BlinnPhongLightColor(vec3 worldLightPos, vec3 viewNormal, vec3 viewFragPosition, bool useNormalMap) {
    vec4 viewLightPos = viewMat * vec4(worldLightPos, 1.0);

    vec3 N = normalize(viewNormal);
    vec3 L = viewLightPos.xyz - viewFragPosition;
    vec3 V = normalize(- viewFragPosition);
    L = normalize(L);
    vec3 H = normalize(L + V);

    vec3 ambient = Ia * Ka;
	vec3 diffuse = Id * max(dot(N, L), 0) * Kd;
	vec3 specular = Is * pow(max(dot(N, H), 0), shininess) * Ks;
    

    if(useNormalMap){
        vec3 normal_tex = texture(normalTexture_map, fs_in.texcoord).rgb;  
        vec3 world_tangent = texture(tangent_map, fs_in.texcoord).rgb;

        vec3 T = world_tangent;
        vec3 B = cross(N, T);

        vec3 L_t = normalize(vec3(dot(L, T), dot(L, B), dot(L, N)));
        vec3 V_t = normalize(vec3(dot(V, T), dot(V, B), dot(V, N)));
        vec3 H_t = normalize(L_t+ V_t);
	    vec3 N_t = vec3(0.0, 0.0, 1.0);  
        
        N_t = normalize(normal_tex * 2.0 - vec3(1.0));
	    diffuse = Id * max(dot(N_t, L_t), 0) * Kd;
	    specular = Is * pow(max(dot(N_t, H_t), 0), shininess) * Ks;
    }

    return ambient + diffuse + specular;
}

float DirLightShadow(vec3 fragPos){
    vec4 shadow_coord = shadow_matrix * vec4(fragPos, 1.0);
    return 1.0 - textureProj(dirLightShadow_map, shadow_coord);
}

float PointLightShadow(vec3 fragPos){

    // Get vector between fragment position and light position
    vec3 fragToLight = fragPos - pointLightPos;

    // Use the light to fragment vector to sample from the depth map    
    float closestDepth = texture(pointLightShadow_map, fragToLight).r;
    
    // It is currently in linear range between [0,1]. Re-transform back to original value
    closestDepth *= 10.0;

    // Now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);

    // Now test for shadows
    float bias = 0.05; 
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}

// view_position = position of frag in view space
vec3 AreaLightColor(vec4 view_position, vec3 normal, vec3 viewDir, vec3 diffuseFactor){
    float rect_h = 1.0;
    float rect_w = 1.0;
    vec3 r = reflect(-viewDir, normal);
    vec3 lightColor = vec3(0.8, 0.6, 0.0);
    vec3 arealight_worldpos = areaLightPos;

    // left upper
    vec4 light_cornerPos_view0
        = viewMat * vec4( arealight_worldpos + vec3( - rect_w * 0.5, rect_h * 0.5, 0), 1.0);
    // left bottom
    vec4 light_cornerPos_view1 
        = viewMat * vec4( arealight_worldpos + vec3( - rect_w * 0.5, - rect_h * 0.5, 0), 1.0);
    // right bottom
    vec4 light_cornerPos_view2 
        = viewMat * vec4( arealight_worldpos + vec3( rect_w * 0.5, - rect_h * 0.5, 0), 1.0);
    // right upper
    vec4 light_cornerPos_view3 
        = viewMat * vec4( arealight_worldpos + vec3( rect_w * 0.5, rect_h * 0.5, 0), 1.0);
    // light center in view sapce
    vec4 light_centerPos = viewMat * vec4(arealight_worldpos, 1.0);

    vec3 right = (light_cornerPos_view3  - light_cornerPos_view0).xyz;
    vec3 up = (light_cornerPos_view0  - light_cornerPos_view1).xyz;
    right = normalize(right);
    up = normalize(up);

    vec3 lightDir = normalize(cross(right, up));


    vec3 v0 = light_cornerPos_view0.xyz - view_position.xyz;
    vec3 v1 = light_cornerPos_view1.xyz - view_position.xyz;
    vec3 v2 = light_cornerPos_view2.xyz - view_position.xyz;
    vec3 v3 = light_cornerPos_view3.xyz - view_position.xyz; 

    float facingCheck 
        = dot( v0, cross( ( light_cornerPos_view2.xyz - light_cornerPos_view0.xyz ).xyz, ( light_cornerPos_view1.xyz - light_cornerPos_view0.xyz ).xyz ) );
    
    if (facingCheck > 0.0) 
    {
        return vec3(0.0, 0.0, 0.0);
    }

    vec3 n0 = normalize ( cross (v0 , v1 ));
    vec3 n1 = normalize ( cross (v1 , v2 ));
    vec3 n2 = normalize ( cross (v2 , v3 ));
    vec3 n3 = normalize ( cross (v3 , v0 ));
    float g0 = acos ( dot (-n0 , n1 ));
    float g1 = acos ( dot (-n1 , n2 ));
    float g2 = acos ( dot (-n2 , n3 ));
    float g3 = acos ( dot (-n3 , n0 ));

    float solidAngle = g0 + g1 + g2 + g3 - 2.0 * PI;

    float totalWeight  = solidAngle  * 1.0 * (
	    clamp ( dot( normalize ( v0 ) , normal ), 0.0, 1.0 ) +
	    clamp ( dot( normalize ( v1 ) , normal ), 0.0, 1.0 )+
	    clamp ( dot( normalize ( v2 ) , normal ), 0.0, 1.0 )+
	    clamp ( dot( normalize ( v3 ) , normal ), 0.0, 1.0 )+
	    clamp ( dot( normalize ( light_centerPos.xyz - view_position.xyz ) , normal ), 0.0, 1.0 ) );

    float d0 = length(v0);
    float d1 = length(v1);
    float d2 = length(v2);
    float d3 = length(v3);
    float dCenter = length(light_centerPos.xyz - view_position.xyz);

    float weight0 = pow(d0, 0.5); 
    float weight1 = pow(d1, 0.5);
    float weight2 = pow(d2, 0.5);
    float weight3 = pow(d3, 0.5);
    float weightCenter = pow(dCenter, 0.5);
    float NoL = solidAngle * totalWeight ;


    vec3 intersectPoint = CalculatePlaneIntersection(view_position.xyz, r, lightDir, light_centerPos.xyz);

    vec3 intersectionVector = intersectPoint - light_centerPos.xyz;
    vec2 intersectPlanePoint = vec2(dot(intersectionVector, right), dot(intersectionVector, up));
    vec2 nearest2DPoint = vec2(clamp(intersectPlanePoint.x, -rect_w*0.5, rect_w*0.5), clamp(intersectPlanePoint.y, -rect_h*0.5, rect_h*0.5));

    vec3 specularFactor = vec3(0,0,0);
    float specularAmount = dot(r,lightDir);
    float roughness = 0.2;
    float surfaceSpec = 1;
    if (specularAmount > 0.0)
    {
         float specFactor = 1.0 - clamp(length(nearest2DPoint - intersectPlanePoint) * pow((1.0 - roughness), 2) * 32.0, 0.0, 1.0);
         specularFactor += surfaceSpec * specFactor * specularAmount * NoL;
    }	

    float lightRadius = 5.0;
    vec3 nearestPoint = light_centerPos.xyz + (right * nearest2DPoint.x + up * nearest2DPoint.y);
    float dist = distance(view_position.xyz, light_centerPos.xyz);
    float falloff = 1.0 - clamp(dist/lightRadius, 0.0, 1.0);	
    falloff = 1.0 / (1.0 + dist * dist);

    float luminosity = 1.0;
    vec3 light = (  diffuseFactor * NoL )  * falloff * lightColor * 1.0;
    return light;
}

vec3 CalculatePlaneIntersection(vec3 viewPosition, vec3 reflectionVector, vec3 lightDirection, vec3 rectangleLightCenter)
{
   return viewPosition + reflectionVector * (dot(lightDirection,rectangleLightCenter-viewPosition)/dot(lightDirection,reflectionVector));
}