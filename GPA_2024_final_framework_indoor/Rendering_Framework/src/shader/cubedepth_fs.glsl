#version 430 core

in vec4 FragPos;

uniform vec3 pointLightPos;
uniform float far_plane;
layout (location = 0) out vec4 frag_color;
void main()
{
    // get distance between fragment and light source
    float lightDistance = length(FragPos.xyz - pointLightPos);

    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / far_plane;

    // Write this as modified depth
    gl_FragDepth = lightDistance;

}