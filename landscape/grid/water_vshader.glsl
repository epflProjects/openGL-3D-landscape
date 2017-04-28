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
    gl_Position = MVP * vec4(position.x, 0.0, -position.y, 1.0);
}
