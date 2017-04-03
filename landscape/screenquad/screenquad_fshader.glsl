#version 330

in vec2 uv;

out vec3 color;

uniform int pass;

uniform sampler2D tex;
uniform float tex_width;
uniform float tex_height;
uniform float[] kernel_g;
uniform float variance;

float rgb_2_luma(vec3 c) {
    return 0.3*c[0] + 0.59*c[1] + 0.11*c[2];
}

void oneDimBlur(bool isVert) {
    // standard deviation
    float std = 2 + variance;
    if (std < 0.1f) {
      std = 0.1f;
    }

    vec3 color_tot = vec3(0,0,0);
    float weight_tot = 0;
    int SIZE = 1 + 2 * 3 * int(ceil(std));

    //gaussian blur algorithm adapted for one dimension.
    for(int i=-SIZE; i<=SIZE; i++){
        float w = exp(-(i*i)/(2.0*std*std));
        float x = isVert ? 0 : i/tex_width;
        float y = isVert ? i/tex_height : 0;

        vec3 neigh_color = texture(tex, uv+vec2(x,y)).rgb;
        color_tot += w * neigh_color;
        weight_tot += w;
    }
    color = color_tot / weight_tot;
}

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
