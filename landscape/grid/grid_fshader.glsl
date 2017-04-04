#version 330

in vec2 uv;
in vec4 vpoint_mv;

out vec3 color;

uniform sampler2D tex;

void main() {
    vec3 vcolor = vec3(0.0f, 0.0f, 0.0f);

    // compute triangle normal using dFdx and dFdy
    vec3 fragVertexEc = vpoint_mv.xyz;
    vec3 x = dFdx(fragVertexEc);
    vec3 y = dFdy(fragVertexEc);
    vec3 triangle_normal = normalize(cross(x, y));

    color = triangle_normal;
}
