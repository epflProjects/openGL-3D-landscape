#version 330

in vec2 uv;

out vec3 color;

uniform sampler2D heightmap_tex;
uniform float tex_width;
uniform float tex_height;
uniform int[] permutation;

float GenerateFrameTexture(float position) {
    return (float)permutation[p * 256] / 255.0f;
}

void main() {
    color = texture(heightmap_tex,uv).rgb;
}
