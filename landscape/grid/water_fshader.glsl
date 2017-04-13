#version 330

in vec2 uv;
in vec4 vpoint_mv;
in vec3 light_dir;
in float height;

out vec4 color;

uniform sampler2D tex;

vec3 hexToFloatColor(vec3 hex){
	return hex/255.0f;
}

void main() {
    color = vec4(hexToFloatColor(vec3(45,47,220)), 0.7f);
}
