#version 330

in vec2 uv;

out vec3 color;

uniform sampler2D skyTex;

void main() {
    color = texture(skyTex, uv).rgb;
}
