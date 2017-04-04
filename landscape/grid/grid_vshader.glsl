#version 330

in vec2 position;

out vec2 uv;
out vec4 vpoint_mv;

uniform mat4 MVP;
uniform float time;
uniform sampler2D heightmap_tex;

void main() {
    uv = (position + vec2(1.0, 1.0)) * 0.5;
    // convert the 2D position into 3D positions that all lay in a horizontal
    // plane.
    float newX = position.x;
    float newY = position.y;
    float height = texture(heightmap_tex, vec2(newX, newY)).z; // TODO not sure

    vpoint_mv = MVP * vec4(vec3(newX, newY, height), 1.0); // TODO not sure

    vec3 pos_3d = vec3(newX, height, -newY);
    gl_Position = MVP * vec4(pos_3d, 1.0);
}
