#version 330

in vec2 uv;
in vec4 vpoint_mv;
in vec3 light_dir;
in float height;

out vec3 color;

uniform sampler2D tex;

vec3 hexToFloatColor(vec3 hex){
	return hex/255.0f;
}

/**
 * According to the current height, return the correct color
 * of the field in HEX form.
 *
 */
vec3 heightColor(float height){
	//TODO : calculate borns of height according to max/min value of height. (bc magic number is bad.)
	vec3 sea = vec3(0, 61, 218);
	vec3 coast = vec3(216, 204, 96);
	vec3 land = vec3(49, 160, 64);
	vec3 forest = vec3(18,91,42);
	vec3 mountain = vec3(142, 142, 142);
	vec3 noColor = vec3(0, 0, 0);

	if(height <= 0.0f){
		return sea;
	}else if(height <= 0.01f){
		return coast;
	}else if(height <= 0.03){
		return land;
	}else if(height <= 0.04){
		return forest;
	}else if(height <= 0.045){
		return noColor;
	}else{
		return mountain;
	}
}

void main() {
	//source : hw3_flatshader.
    vec3 vcolor = vec3(0.0f, 0.0f, 0.0f);
    vec3 Ld = vec3(1.0f, 1.0f, 1.0f);

    // compute triangle normal using dFdx and dFdy
    vec3 fragVertexEc = vpoint_mv.xyz;
    vec3 x = dFdx(fragVertexEc);
    vec3 y = dFdy(fragVertexEc);
    vec3 triangle_normal = normalize(cross(x, y));

    //compute diffuse term. (source : part of flat_shading hw3)
    vec3 n = normalize(triangle_normal);
    vec3 l = normalize(light_dir);
    float lambert = dot(n, l);
    if (lambert > 0.0f) {
      vcolor += lambert * Ld;
    }
    color = vcolor + hexToFloatColor(heightColor(height));
}
