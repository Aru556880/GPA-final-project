#version 430 core

layout (location = 0) out vec4 frag_color;

layout (binding = 0) uniform sampler2D scene;
layout (binding = 1) uniform sampler2D bright_scene;

in VS_OUT
{
    vec2 texcoord;
} fs_in;

uniform vec2 screen_light_pos;

const int NUM_SAMPLES = 100;
const float exposure = 0.2f;
const float decay = 0.96815f;
const float weight = 0.58767f;
const float density = 0.926f;

void main() {
    vec2 delta_texcoord = (fs_in.texcoord - screen_light_pos);
    delta_texcoord *= density / float(NUM_SAMPLES);

    vec3 color = texture(bright_scene, fs_in.texcoord).rgb;

    float illumination_decay = 1.0f;

    vec2 sample_coord = fs_in.texcoord;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        sample_coord -= delta_texcoord;
        vec3 sample_color = texture(bright_scene, sample_coord).rgb;
        sample_color *= illumination_decay * weight;
        color += sample_color;
        illumination_decay *= decay;
    }

    frag_color = vec4(color * exposure, 1.0) * 0.4 + texture(scene, fs_in.texcoord) * 0.6;
}
