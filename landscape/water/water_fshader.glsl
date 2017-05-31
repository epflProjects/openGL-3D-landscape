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

    // compute triangle normal using dFdx and dFdy
    vec3 fragVertexEc = vpoint_mv.xyz;
    vec3 x = dFdx(fragVertexEc);
    vec3 y = dFdy(fragVertexEc);
    vec3 triangle_normal = normalize(cross(x, y));

    //compute diffuse term. (source : part of flat_shading hw3)
    vec3 diffuse_light = vec3(0.0f,0.0f,0.0f);
    vec3 Ld = vec3(1.0f, 1.0f, 1.0f);
    vec3 l = normalize(light_dir);
    float lambert = dot(triangle_normal, l);
    if (lambert > 0.0f) {
      diffuse_light += (lambert * Ld) * 0.5f;
    }


    //we need to adapt the view's mirrored coordinate to the ones of the water textures, to avoid distorsion in the .
    float _u = gl_FragCoord.x / window_width + 0.01f*triangle_normal.x;

    float _v = gl_FragCoord.y / window_height + 0.01f*triangle_normal.y;

    color = vec4(0.35f * diffuse_light + mix(hexToFloatColor(vec3(104.0f,128.0f,156.0f)), texture(tex_mirror, vec2(_u,_v)).rgb, vec3(.35)), 0.8f);
    //color = vec4(texture(tex_mirror, vec2(_u,_v)).rgb, 0.8f);
}
