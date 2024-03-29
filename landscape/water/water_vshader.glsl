#version 330

in vec2 position;

out vec2 uv;
out vec4 vpoint_mv;
out vec3 light_dir;
out float height;

uniform mat4 MVP;
uniform float time;

#define PI 3.14159265358979323846264338328

#define WAVE_NBR 6

void light_handler(){

}


void main() {
    //gl_Position = MVP * vec4(position.x, 0.0, -position.y, 1.0);
    uv = (position + vec2(1.0, 1.0)) * 0.5;

    // convert the 2D position into 3D positions that all lay in a horizontal
    // plane.
    float height = 0;
    float newX = position.x;
    float newY = position.y;

    float A[WAVE_NBR] = float[](0.0005, 0.00016, 0.00014, 0.00017, 0.00014, 0.00018); 
    vec2 D[WAVE_NBR] = vec2[](vec2(1.0, 1.0), vec2(0.0, 0.9), vec2(-0.4, 0.6), vec2(-0.1, -0.8), vec2(-0.9, -1.0), vec2(0.4, -0.5));
    float L[WAVE_NBR] = float[](0.006, 0.008, 0.009, 0.008, 0.01, 0.007);
    float S[WAVE_NBR] = float[](0.2, 0.15, 0.19, 0.23, 0.25, 0.22);
    
    float x = uv[0];
    float y = uv[1];


    //Using Gernster Wave equation for 5 different arbitrary defined vawe.
    for(int i = 0; i < WAVE_NBR; ++i){
    	float wi = 2*PI/L[i];
    	float phi = S[i]*wi;
    	float sinusoidArg = wi * (x*D[i][0] + y*D[i][1]) + time;
    	float Qi = 1/(wi*A[i]*3.5);
    	height += A[i] * sin(sinusoidArg);
    	newX += Qi*A[i]*D[i][0] * cos(sinusoidArg);
    	newX += Qi*A[i]*D[i][1] * cos(sinusoidArg);
    }

    vec3 pos_3d = vec3(newX, height, -newY);

    //for diffuse lightning on frag shader.
    vec3 light_pos = vec3(27.0f, 34.0f, 2.0f);
    vpoint_mv = MVP * vec4(vec3(newX, newY, height), 1.0);
    light_dir = light_pos - vpoint_mv.xyz;

    gl_Position = MVP * vec4(pos_3d, 1.0);
}
