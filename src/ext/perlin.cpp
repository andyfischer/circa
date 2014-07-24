// See perlin.h for license information

#include "common_headers.h"
#include <stdlib.h>

#include <stdio.h>
#include <math.h>

#include "perlin.h"
#include "rand.h"

namespace circa {

u8 gPerms[512];

void init_permutation_table()
{
    // For now, use a throwaway RandState and share the permutation table across process.
    RandState randState;
    rand_init(&randState, 0);
    for( int t = 0; t < 256; ++t ) {
        gPerms[t] = gPerms[t + 256] = rand_next_int(&randState) & 255;
    }
}

void perlin_init()
{
    init_permutation_table();
}

static inline float fade( float t ) { return t * t * t * (t * (t * 6 - 15) + 10); }
static inline float dfade( float t ) { return 30.0f * t * t * ( t * ( t - 2.0f ) + 1.0f ); }
inline float nlerp(float t, float a, float b) { return a + t * (b - a); }

/////////////////////////////////////////////////////////////////////////////////////////////////
// grad

float grad( int32_t hash, float x )
{
    int32_t h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
    float    u = h<8 ? x : 0,                 // INTO 12 GRADIENT DIRECTIONS.
            v = h<4 ? 0 : h==12||h==14 ? x : 0;
    return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}

float grad( int32_t hash, float x, float y )
{
    int32_t h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
    float    u = h<8 ? x : y,                 // INTO 12 GRADIENT DIRECTIONS.
            v = h<4 ? y : h==12||h==14 ? x : 0;
    return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}

float grad( int32_t hash, float x, float y, float z )
{
    int32_t h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
    float u = h<8 ? x : y,                 // INTO 12 GRADIENT DIRECTIONS.
         v = h<4 ? y : h==12||h==14 ? x : z;
    return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// fBm
float perlin_fbm(int octaves, float x)
{
    float result = 0.0f;
    float amp = 0.5f;

    for( uint8_t i = 0; i < octaves; i++ ) {
        result += perlin_noise( x ) * amp;
        x *= 2.0f;
        amp *= 0.5f;
    }

    return result / 2 + 0.5;
}

float perlin_fbm(int octaves, float x, float y)
{
    float result = 0.0f;
    float amp = 0.5f;

    for( uint8_t i = 0; i < octaves; i++ ) {
        result += perlin_noise( x, y ) * amp;
        x *= 2.0f; y *= 2.0f;
        amp *= 0.5f;
    }

    return result / 2 + 0.5;
}

float perlin_fbm(int octaves, float x, float y, float z)
{
    float result = 0.0f;
    float amp = 0.5f;

    for( uint8_t i = 0; i < octaves; i++ ) {
        result += perlin_noise( x, y, z ) * amp;
        x *= 2.0f; y *= 2.0f; z *= 2.0f;
        amp *= 0.5f;
    }

    return result / 2 + 0.5;
}

void perlin_dfbm(int octaves, float x, float y, float* xout, float* yout)
{
    float amp = 0.5f;

    *xout = 0;
    *yout = 0;

    for( uint8_t i = 0; i < octaves; i++ ) {
        float dnX, dnY;
        perlin_dnoise(x, y, &dnX, &dnY);
        dnX *= amp;
        dnY *= amp;
        *xout += dnX;
        *yout += dnY;
        x *= 2.0f; y *= 2.0f;
        amp *= 0.5f;
    }
}

void perlin_dfbm(int octaves, float x, float y, float z, float* xout, float* yout, float* zout)
{
    *xout = 0;
    *yout = 0;
    *zout = 0;

    float amp = 0.5f;

    for( uint8_t i = 0; i < octaves; i++ ) {
        float dnX, dnY, dnZ;
        perlin_dnoise( x, y, z, &dnX, &dnY, &dnZ );

        dnX *= amp;
        dnY *= amp;
        dnZ *= amp;
        *xout += dnX;
        *yout += dnY;
        *zout += dnZ;

        x *= 2.0f; y *= 2.0f; z *= 2.0f;
        amp *= 0.5f;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// noise
float perlin_noise( float x ) 
{
    int32_t X = ((int32_t)floorf(x)) & 255;
    x -= floorf(x);
    float u = fade( x );
    int32_t A = gPerms[X], AA = gPerms[A], B = gPerms[X+1], BA = gPerms[B];

    return nlerp( u, grad( gPerms[AA  ], x ), grad( gPerms[BA], x-1 ) );
}

float perlin_noise( float x, float y ) 
{
    int32_t X = ((int32_t)floorf(x)) & 255, Y = ((int32_t)floorf(y)) & 255;
    x -= floorf(x); y -= floorf(y);
    float    u = fade( x ), v = fade( y );
    int32_t A = gPerms[X  ]+Y, AA = gPerms[A], AB = gPerms[A+1],
    B = gPerms[X+1]+Y, BA = gPerms[B], BB = gPerms[B+1];

    return nlerp(v, nlerp(u, grad(gPerms[AA  ], x  , y   ),
                             grad(gPerms[BA  ], x-1, y   )),
                     nlerp(u, grad(gPerms[AB  ], x  , y-1   ),
                             grad(gPerms[BB  ], x-1, y-1   )));
}

float perlin_noise( float x, float y, float z ) 
{
    // These floors need to remain that due to behavior with negatives.
    int32_t X = ((int32_t)floorf(x)) & 255, Y = ((int32_t)floorf(y)) & 255, Z = ((int32_t)floorf(z)) & 255;
    x -= floorf(x); y -= floorf(y); z -= floorf(z);
    float    u = fade(x), v = fade(y), w = fade(z);
    int32_t A = gPerms[X  ]+Y, AA = gPerms[A]+Z, AB = gPerms[A+1]+Z,
    B = gPerms[X+1]+Y, BA = gPerms[B]+Z, BB = gPerms[B+1]+Z;

    float a = grad(gPerms[AA  ], x  , y  , z   );
    float b = grad(gPerms[BA  ], x-1, y  , z   );
    float c = grad(gPerms[AB  ], x  , y-1, z   );
    float d = grad(gPerms[BB  ], x-1, y-1, z   );
    float e = grad(gPerms[AA+1], x  , y  , z-1 );
    float f = grad(gPerms[BA+1], x-1, y  , z-1 );
    float g = grad(gPerms[AB+1], x  , y-1, z-1 );
    float h = grad(gPerms[BB+1], x-1, y-1, z-1 );

    return    nlerp(w, nlerp( v, nlerp( u, a, b ),
                             nlerp( u, c, d ) ),
                    nlerp(v, nlerp( u, e, f ),
                             nlerp( u, g, h ) ) );    
}

// Credit for the ideas for analytical Perlin derivatives below are due to Iñigo Quílez
void perlin_dnoise( float x, float y, float* xout, float* yout ) 
{
    int32_t X = ((int32_t)x) & 255, Y = ((int32_t)y) & 255;
    x -= floorf(x); y -= floorf(y);
    float u = fade( x ), v = fade( y );
    float du = dfade( x ), dv = dfade( y );
    int32_t A = gPerms[X  ]+Y, AA = gPerms[A]+0, AB = gPerms[A+1]+0,
        B = gPerms[X+1]+Y, BA = gPerms[B]+0, BB = gPerms[B+1]+0;

    if( du < 0.000001f ) du = 1.0f;
    if( dv < 0.000001f ) dv = 1.0f;

    float a = grad( gPerms[AA], x  , y   );
    float b = grad( gPerms[BA], x-1, y   );
    float c = grad( gPerms[AB], x  , y-1   );
    float d = grad( gPerms[BB], x-1, y-1   );
    
    const float k1 =   b - a;
    const float k2 =   c - a;
    const float k4 =   a - b - c + d;

    *xout = du * ( k1 + k4 * v );
    *yout = dv * ( k2 + k4 * u );
}

void perlin_dnoise( float x, float y, float z, float* xout, float* yout, float* zout )
{
    int32_t X = ((int32_t)floorf(x)) & 255, Y = ((int32_t)floorf(y)) & 255, Z = ((int32_t)floorf(z)) & 255;
    x -= floorf(x); y -= floorf(y); z -= floorf(z);
    float u = fade( x ), v = fade( y ), w = fade( z );
    float du = dfade( x ), dv = dfade( y ), dw = dfade( z );
    int32_t A = gPerms[X  ]+Y, AA = gPerms[A]+Z, AB = gPerms[A+1]+Z,
        B = gPerms[X+1]+Y, BA = gPerms[B]+Z, BB = gPerms[B+1]+Z;

    if( du < 0.000001f ) du = 1.0f;
    if( dv < 0.000001f ) dv = 1.0f;
    if( dw < 0.000001f ) dw = 1.0f;    

    float a = grad( gPerms[AA  ], x  , y  , z   );
    float b = grad( gPerms[BA  ], x-1, y  , z   );
    float c = grad( gPerms[AB  ], x  , y-1, z   );
    float d = grad( gPerms[BB  ], x-1, y-1, z   );
    float e = grad( gPerms[AA+1], x  , y  , z-1 );
    float f = grad( gPerms[BA+1], x-1, y  , z-1 );
    float g = grad( gPerms[AB+1], x  , y-1, z-1 );
    float h = grad( gPerms[BB+1], x-1, y-1, z-1 );

    const float k1 =   b - a;
    const float k2 =   c - a;
    const float k3 =   e - a;
    const float k4 =   a - b - c + d;
    const float k5 =   a - c - e + g;
    const float k6 =   a - b - e + f;
    const float k7 =  -a + b + c - d + e - f - g + h;

    *xout = du * ( k1 + k4*v + k6*w + k7*v*w );
    *yout = dv * ( k2 + k5*w + k4*u + k7*w*u );
    *zout = dw * ( k3 + k6*u + k5*v + k7*u*v );
}

} // namespace circa
