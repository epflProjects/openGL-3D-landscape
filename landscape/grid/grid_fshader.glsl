#version 330

in vec2 uv;
in vec4 vpoint_mv;

out vec3 color;

uniform sampler2D tex;

float height(vec2 xy){
	return texture(tex, xy).z;
}

void main() {
    
    //still need some adaptation and engineering, the result is fare from beign perfect.
    vec3 vcolor = vec3(0.0f, 0.0f, 0.0f);
    vec3 off = vec3(1.0f, 1.0f, 0.0f);
    vec3 P = vpoint_mv.xyz;

	float hL = height(P.xy - off.xz);
	float hR = height(P.xy + off.xz);
	float hD = height(P.xy - off.zy);
	float hU = height(P.xy + off.zy);

	vec3 N = vec3(hL-hR, hD - hU, 2.0f);
	color = normalize(N);

    /*// compute triangle normal using dFdx and dFdy
    vec3 fragVertexEc = vpoint_mv.xyz;
    vec3 x = dFdx(fragVertexEc);
    vec3 y = dFdy(fragVertexEc);
    vec3 triangle_normal = normalize(cross(x, y));

    color = triangle_normal;*/
}
