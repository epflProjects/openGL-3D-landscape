#version 330

in vec2 uv;
in vec4 vpoint_mv;
in vec3 light_dir;
in float height;

out vec3 color;

uniform sampler2D tex;

/**
 * According to the current height, return the correct color
 * of the field.
 *
 */
vec3 heightColor(float height){
	//TODO : calculate borns of height according to max/min value of height. (bc magic number is bad.)
	vec3 blue = vec3(0.0f, 0.0f, 1.0f);
	vec3 green = vec3(0.0f, 1.0f, 0.0f);
	vec3 noColor = vec3(0.0f, 0.0f, 0.0f);
	if(height <= 0.0f){
		return blue;
	}else if(height <= 0.01f)
		return green;
	else{
		return noColor;
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
    color = vcolor; //+ heightColor(height);
}
