#version 330

in vec2 position;

out vec2 uv;
out vec4 vpoint_mv;
out vec3 light_dir;

uniform mat4 MVP;
uniform float time;
uniform sampler2D heightmap_tex;

void main() {
	vec3 light_pos = vec3(1.0f, 1.0f, 2.0f);

    uv = (position + vec2(1.0, 1.0)) * 0.5;
    float newX = (position.x + 1) * 0.5;
    float newY = (position.y + 1) * 0.5;
    float height = (texture(heightmap_tex, vec2(newX, newY)).z)/20; // TODO not sure

    vpoint_mv = MVP * vec4(vec3(newX, newY, height), 1.0); // TODO not sure
    
    //for diffuse lightning on frag shader.
    light_dir = light_pos - vpoint_mv.xyz;

    vec3 pos_3d = vec3(newX, height, -newY);
    gl_Position = MVP * vec4(pos_3d, 1.0);
}
