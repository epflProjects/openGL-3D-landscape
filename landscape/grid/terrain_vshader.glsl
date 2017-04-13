#version 330

in vec2 position;

out vec2 uv;
out vec4 vpoint_mv;
out vec3 light_dir;
out float height;

uniform mat4 MVP;
uniform float time;
uniform sampler2D heightmap_tex;

void main() {
	vec3 light_pos = vec3(27.0f, 34.0f, 2.0f);

    uv = (position + vec2(1.0, 1.0)) * 0.5;
    float newX = position.x;
    float newY = position.y;
    height = (texture(heightmap_tex, uv).r)/3.0f;

    vpoint_mv = MVP * vec4(vec3(newX, newY, height), 1.0); // TODO not sure

    //for diffuse lightning on frag shader.
    light_dir = light_pos - vpoint_mv.xyz;

    vec3 pos_3d = vec3(newX, height, -newY);
    gl_Position = MVP * vec4(pos_3d, 1.0);
}
