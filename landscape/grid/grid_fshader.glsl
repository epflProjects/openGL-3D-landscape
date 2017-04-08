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
 * Will construct a color that is in between two colors (currColor and nextColor)
 * according to the current height of the texture. Used to make smooth transition between 
 * the colors of two types of landfield.
 * It is kind of like a mapping.
 *
 */
vec3 colorGraduator(vec3 currColor, vec3 nextColor, float height, float prevHeight, float nexHeight){
	float rGrad = nextColor.x - currColor.x;
	float gGrad = nextColor.y - currColor.y;
	float bGrad = nextColor.z - currColor.z;

	float relH = height - prevHeight;
	float heightInterval = nexHeight - prevHeight;
	float percentage = relH / heightInterval;

	return currColor + vec3(rGrad * percentage, gGrad * percentage, bGrad * percentage);
}

#define LAND_TYPES_NBR 6
vec3 sea = vec3(0, 61, 218);
vec3 coast = vec3(216, 204, 96);
vec3 land = vec3(49, 160, 64);
vec3 forest = vec3(44,86,99);
vec3 mountain = vec3(142, 142, 142);
vec3 noColor = vec3(0, 0, 0);

vec3 landColors[LAND_TYPES_NBR] = vec3[](sea, coast, land, forest, noColor, mountain);
float landLimits[LAND_TYPES_NBR - 1] = float[](0.0f, 0.01f, 0.03f, 0.045f, 0.055f);


/**
 * According to the current height, return the correct color
 * of the field in HEX form.
 */
vec3 heightColor(float height){

	if(height <= landLimits[0]){
		return landColors[0];
	}

	for(int i = 1; i < LAND_TYPES_NBR - 1; ++i){
		if(height <= landLimits[i]){
			return colorGraduator(landColors[i], landColors[i+1], height, landLimits[i-1], landLimits[i]);
		}
	}

	return landColors[LAND_TYPES_NBR - 1];
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
      vcolor += (lambert * Ld) * 0.5f;
    }
    color = vcolor + hexToFloatColor(heightColor(height));
}
