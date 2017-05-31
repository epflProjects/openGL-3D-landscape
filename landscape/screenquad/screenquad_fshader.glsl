#version 330

// if 1 the shader uses Perlin Noise, otherwise uses Simplex Noise

in vec2 uv;
in vec3 vp;

out vec3 color;

uniform sampler1D permutation_tex;
uniform float tex_width;
uniform float tex_height;
uniform float H;
uniform float lacunarity;
uniform float exponent_array[10];

uniform bool PERLIN_NOISE;

#if PERLIN_NOISE
// Perlin Noise
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

float perlin_noise(float x, float y, float z) {
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
#else
// Simplex Noise
//
// Description : Array and textureless GLSL 2D/3D/4D simplex
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
//

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r) {
  return 1.79284291400159 - 0.85373472095314 * r;
}

float simplex_noise(vec3 v) {
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289(i);
  vec4 p = permute( permute( permute(
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1),
                                dot(p2,x2), dot(p3,x3) ) );
  }
#endif

float RMF (vec3 point, float octaves){
  float result, signal, weight, frequency = 0.6, H = 1.0, offset = 1.0, gain = 2.0;
  int i;
  for (i=0; i<octaves; i++) {
      frequency *= lacunarity;
    }

    #if PERLIN_NOISE
    signal = perlin_noise(point.x, point.y, point.z);
    #else
    signal = simplex_noise(vec3(point.x, point.y, point.z));
    #endif

    if (signal < 0.0) {
        signal = -signal;
    }

    signal = offset - signal;
    signal *= signal;
    result = signal;
    weight = 1.0;

    for (i = 1; i < octaves; ++i) {
        point.x *= lacunarity;
        point.y *= lacunarity;
        point.z *= lacunarity;
        weight = signal * gain;

            if (weight > 1.0) {
                weight = 1.0;
            }

            if (weight < 0.0) {
                weight = 0.0;
            }

            #if PERLIN_NOISE
            signal = perlin_noise(point.x, point.y, point.z);
            #else
            signal = simplex_noise(vec3(point.x, point.y, point.z));
            #endif
            //signal = worley_noise(vec3(point.x, point.y, point.z));

            if (signal < 0.0) {
                signal = -signal;
            }

            signal = offset - signal;
            signal *= signal;

            signal *= weight;
            result += signal * exponent_array[i];
    }
    return (result - 1.4) / 2.0;
}

//Do the fractal brownian motion.
//Need to have exponent_array initialized according to frequency and lacunarity. (done on CPU now)
float fBm (vec3 point, float octaves){
   float value = 0.0f;
   int i;
   for(i = 0; i < octaves; ++i) {
       #if PERLIN_NOISE
       value += perlin_noise(point.x, point.y, point.z) * exponent_array[i];
       #else
       value += simplex_noise(point) * exponent_array[i];
       #endif
       point.x *= lacunarity;
       point.y *= lacunarity;
       point.z *= lacunarity;
   }

   float remainder = octaves - int(octaves);
   if(remainder == 0.0) {
       #if PERLIN_NOISE
       value += remainder * perlin_noise(point.x, point.y, point.z) * exponent_array[i];
       #else
       value += remainder * simplex_noise(point) * exponent_array[i];
       #endif
   }
   return value;
}

void main() {
    float octaves = log(tex_height)/log(2) - 2;
    color = vec3(RMF(vec3(uv.x*3, uv.y*3, 0), octaves));
}
