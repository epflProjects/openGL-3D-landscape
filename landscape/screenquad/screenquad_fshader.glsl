#version 330

in vec2 uv;

out vec3 color;

uniform int pass;

uniform sampler2D heightmap_tex;
uniform float tex_width;
uniform float tex_height;

void main() {
    color = texture(heightmap_tex,uv).rgb;
}
