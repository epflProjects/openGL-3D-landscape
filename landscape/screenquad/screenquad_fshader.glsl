#version 330

in vec2 uv;
in vec3 vp;

out vec3 color;

uniform sampler1D permutation_tex;
uniform float tex_width;
uniform float tex_height;
uniform float H;
uniform float lacunarity;
uniform float exponent_array[10];

// Version from: http://mrl.nyu.edu/~perlin/noise/

int permutation[] = int[]( 151,160,137,91,90,15,
 131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
 190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
 88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
 77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
 102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
 135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
 5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
 223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
 129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
 251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
 49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
 138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
 );

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
    int X = int(floor(x)) & 255;
    int Y = int(floor(y)) & 255;
    int Z = int(floor(z)) & 255;

    x = x - floor(x);
    y = y - floor(y);
    z = z - floor(z);

    float u = fade(x);
    float v = fade(y);
    float w = fade(z);

    int A = permutation[X]+Y;
    int AA = permutation[A]+Z;
    int AB = permutation[A+1]+Z;
    int B = permutation[X+1]+Y;
    int BA = permutation[B]+Z;
    int BB = permutation[B+1]+Z;

    return lerp(w, lerp(v, lerp(u, grad(int(permutation[AA]), x  , y  , z   ),     // AND ADD
                                     grad(int(permutation[BA]), x-1, y  , z   )),  // BLENDED
                             lerp(u, grad(int(permutation[AB]), x  , y-1, z   ),   // RESULTS
                                     grad(int(permutation[BB]), x-1, y-1, z   ))), // FROM  8
                     lerp(v, lerp(u, grad(int(permutation[AA+1]), x  , y  , z-1 ),   // CORNERS
                                     grad(int(permutation[BA+1]), x-1, y  , z-1 )),  // OF CUBE
                             lerp(u, grad(int(permutation[AB+1]), x  , y-1, z-1 ),
                                     grad(int(permutation[BB+1]), x-1, y-1, z-1 ))));
}

//Do the fractinal brownian motion.
//Need to have exponent_array initialized according to frequency and lacunarity. (done on CPU now)
float fBm (vec3 point, float octaves){
    float value = 0.0f;
    int i;
    for(i = 0; i < octaves; ++i){
        value += noise(point.x, point.y, point.z) * exponent_array[i];
        point.x *= lacunarity;
        point.y *= lacunarity;
        point.z *= lacunarity;
    }

    float remainder = octaves - int(octaves);
    if(remainder == 0.0){
        value += remainder * noise(point.x, point.y, point.z) * exponent_array[i];
    }
    return value;
}

void main() {
    float octaves = log(tex_height)/log(2) - 2;
    color = vec3(fBm(vec3(uv.x*10, uv.y*10, 0), octaves));
}
