#version 330

in vec2 uv;
in vec4 vpoint_mv;
in vec3 light_dir;

out vec3 color;

uniform sampler2D tex;

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
      vcolor += lambert * Ld;
    }
    color = vcolor;
}
