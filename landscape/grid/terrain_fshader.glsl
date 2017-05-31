#version 330

#define M_PI 3.14159265359

#define COLOR 0

in vec2 uv;
in vec4 vpoint_mv;
in vec3 light_dir;
in float height;

//vec4 since we need alpha for blending.
out vec4 color;

uniform sampler2D grass_tex; //*10
uniform sampler2D sand_tex; //*60
uniform sampler2D snow_tex; //*30
uniform sampler2D rock_tex; //*10

uniform bool isMirror;

#define LAND_TYPES_NBR 6

float landLimits[LAND_TYPES_NBR - 1] = float[](0.0f, 0.01f, 0.04f, 0.12f, 0.14f);


/**--------------------------------------------------------------------------------**
 *
 *							ColorShader code
 *
 *---------------------------------------------------------------------------------**/

vec3 sea = vec3(0, 61, 218);
vec3 coast = vec3(216, 204, 96);
vec3 land = vec3(49, 160, 64);
vec3 forest = vec3(26, 77, 41);
vec3 mountain = vec3(142, 142, 142);
vec3 noColor = vec3(0, 0, 0);

vec3 landColors[LAND_TYPES_NBR] = vec3[](sea, coast, land, forest, noColor, mountain);

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

vec3 hexToFloatColor(vec3 hex){
	return hex/255.0f;
}

/**--------------------------------------------------------------------------------**
 *
 *							TextureShader code
 *
 *---------------------------------------------------------------------------------**/


const int SAND = 0;
const int GRASS = 1;
const int ROCK = 2;
const int SNOW = 3;

int landTexture[LAND_TYPES_NBR] = int[](SAND, SAND, GRASS, ROCK, SNOW, SNOW);

/*
	Will return the adjusted (according to the UV and texture type) 
	rgba of the texture according to the corresponding selector.
*/
vec4 getTexture(int sel){
	vec4 toRet = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	switch(sel){
		case SAND:
			toRet = texture(sand_tex, uv*60);
			break;
		case GRASS:
			toRet = texture(grass_tex, uv*10);
			break;
		case SNOW:
			toRet = texture(snow_tex, uv*60);
			break;
		case ROCK:
			toRet = texture(rock_tex, uv*10);
			break;
		default:
			toRet = texture(sand_tex, uv*60);
			break;
	}
	return toRet;
}


//kept for historical reason
vec4 getTexture(float height){
	float floor = 0.0f;
	float ceil = 0.12f;
	if(height < floor){
		return texture(sand_tex, uv * 10);
	}else if(height > ceil){
		return texture(grass_tex, uv * 60);
	}else{
		float interval = (ceil - floor);
		float loPercentage = (interval - height)/interval;
		vec4 texLo = texture(sand_tex , uv * 10);
		vec4 texUp = texture(grass_tex, uv * 60);
		return mix(texLo, texUp, 1-loPercentage);
	}
}


/**
 * According to the current height, return the correct color texture of
 *	the terrain.
 */
vec4 heightTexture(float height){

	if(height <= landLimits[0]){
		return getTexture(landTexture[0]);
	}

	for(int i = 1; i < LAND_TYPES_NBR - 1; ++i){
		if(height <= landLimits[i]){

			float interval = landLimits[i] - landLimits[i - 1];
			float deltaHeight = height - landLimits[i - 1];
			float percentage = 1 - ((interval - deltaHeight)/interval);
			return mix(getTexture(landTexture[i-1]), getTexture(landTexture[i]), percentage);
		}
	}

	return getTexture(landTexture[LAND_TYPES_NBR-1]);
}

/**--------------------------------------------------------------------------------**
 *
 *							MainShader code
 *
 *---------------------------------------------------------------------------------**/

void main() {
	//source : hw3_flatshader.
    vec3 diffuse_light = vec3(0.0f, 0.0f, 0.0f);
	vec3 textures_blend = vec3(0.0f, 0.0f, 0.0f);
    vec3 Ld = vec3(1.0f, 1.0f, 1.0f);

    // compute triangle normal using dFdx and dFdy
    vec3 fragVertexEc = vpoint_mv.xyz;
    vec3 x = dFdx(fragVertexEc);
    vec3 y = dFdy(fragVertexEc);
    vec3 triangle_normal = normalize(cross(x, y));

    //compute diffuse term. (source : part of flat_shading hw3)
    vec3 l = normalize(light_dir);
    float lambert = dot(triangle_normal, l);
    if (lambert > 0.0f) {
      diffuse_light += (lambert * Ld) * 0.5f;
    }
    //in mirror mode, to display only what is above the water.
    if(isMirror && height < 0){
    	discard;
    }else{	
	    color = vec4(0.5*diffuse_light + heightTexture(height).rgb, 1);	
    }
#if COLOR
	color = vec4(diffuse_light + hexToFloatColor(heightColor(height)), 1);
#endif
}
