#version 330

in vec2 uv;

out vec3 color;

uniform sampler2D heightmap_tex;
uniform float tex_width;
uniform float tex_height;
uniform int permutation[256];

// Version from: http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter26.html
/*vec3 g[16] = vec3[](vec3(1,1,0), vec3(-1,1,0), vec3(1,-1,0), vec3(-1,-1,0),
            vec3(1,0,1), vec3(-1,0,1), vec3(1,0,-1), vec3(-1,0,-1),
            vec3(0,1,1), vec3(0,-1,1), vec3(0,1,-1), vec3(0,-1,-1),
            vec3(1,1,0), vec3(0,-1,1), vec3(-1,1,0), vec3(0,-1,-1));

vec3 generateGradTexture(int position) {
    return g[position * 16];
}

vec4 generatePermTexture(int position) {
    return vec4(permutation[position * 256] / 255.0f, // TODO not sure
                permutation[position * 256] / 255.0f,
                permutation[position * 256] / 255.0f,
                permutation[position * 256] / 255.0f);
}

vec3 fade(vec3 t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float perm(float x) {
    return tex1D(permSampler, x / 256.0) * 256;
}

float grad(float x, vec3 p) {
    return dot(tex1D(gradSampler, x), p);
}

*/
// Version from: http://mrl.nyu.edu/~perlin/noise/
float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float lerp(float t, float a, float b) {
    return a + t * (b - a);
}

float grad(int hash, float x, float y, float z) {
    int h = hash & 15;
    float u = h<8 ? x : y;
    float v = h<4 ? y : h==12 || h==14 ? x : z;
    return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}

float noise(float x, float y, float z) {
    int X = floor(x) & 255;
    int Y = floor(y) & 255;
    int Z = floor(z) & 255;
    x -= floor(x);
    y -= floor(y);
    z -= floor(z);

    float u = fade(x);
    float v = fade(y);
    float w = fade(z);

    int A = permutation[X]+Y;
    int AA = permutation[A]+Z;
    int AB = permutation[A+1]+Z;
    int B = permutation[X+1]+Y;
    int BA = permutation[B]+Z;
    int BB = permutation[B+1]+Z;

    return lerp(w, lerp(v, lerp(u, grad(permutation[AA  ], x  , y  , z   ),     // AND ADD
                                     grad(permutation[BA  ], x-1, y  , z   )),  // BLENDED
                             lerp(u, grad(permutation[AB  ], x  , y-1, z   ),   // RESULTS
                                     grad(permutation[BB  ], x-1, y-1, z   ))), // FROM  8
                     lerp(v, lerp(u, grad(permutation[AA+1], x  , y  , z-1 ),   // CORNERS
                                     grad(permutation[BA+1], x-1, y  , z-1 )),  // OF CUBE
                             lerp(u, grad(permutation[AB+1], x  , y-1, z-1 ),
                                     grad(permutation[BB+1], x-1, y-1, z-1 ))));
}

void main() {
    color = texture(heightmap_tex,uv).rgb;
}
