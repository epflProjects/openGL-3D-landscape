#version 330

in vec2 position;

out vec2 uv;

#define PI 3.14159265358979323846264338328

uniform mat4 MVP;
uniform float time;

void main() {
    uv = (position + vec2(1.0, 1.0)) * 0.5;

    // convert the 2D position into 3D positions that all lay in a horizontal
    // plane.
    float height = 0;
    float newX = position.x;
    float newY = position.y;

    vec3 pos_3d = vec3(newX, height, -newY);
    gl_Position = MVP * vec4(pos_3d, 1.0);
}
