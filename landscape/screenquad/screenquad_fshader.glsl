#version 330

in vec2 uv;

out vec3 color;

uniform int pass;

uniform sampler2D tex;
uniform float tex_width;
uniform float tex_height;

void main() {
    switch(pass){
        case 0:
            oneDimBlur(false);
            break;
        case 1:
            oneDimBlur(true);
            break;
        default:
            break;
    }
}
