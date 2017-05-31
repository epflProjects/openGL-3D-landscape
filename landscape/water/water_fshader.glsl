#version 330

in vec2 uv;
in vec4 vpoint_mv;
in vec3 light_dir;
in float height;

in vec4 gl_FragCoord;

out vec4 color;

uniform sampler2D tex_mirror;

vec3 hexToFloatColor(vec3 hex){
	return hex/255.0f;
}

void main() {
    float tex_width = 1024.0f;
	 //Querying the inverted View's height and width.
    vec2 window_dims = textureSize(tex_mirror, 0);
    float window_width = window_dims.x;//tex_width; //+ 350;//window_dims.x;
    float window_height = window_dims.y;//tex_width; //+ 250;//window_dims.y;

    //we need to adapt the view's mirrored coordinate to the ones of the water textures, to avoid distorsion in the .
    float _u = gl_FragCoord.x / window_width;

    float _v = gl_FragCoord.y / window_height;
    color = vec4(mix(hexToFloatColor(vec3(104.0f,128.0f,156.0f)), texture(tex_mirror, vec2(_u,_v)).rgb, vec3(.35)), 0.8f);
    //color = vec4(texture(tex_mirror, vec2(_u,_v)).rgb, 0.8f);
}
