#ifndef WF3D_COLOR_H_INCLUDED
#define WF3D_COLOR_H_INCLUDED

#include <stdint.h>

#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>

#include <math.h>

typedef struct
{
    float rgba[4];

} wf3d_color;

//Set a color
wf3d_color* wf3d_color_SetRGBA(wf3d_color* color, uint8_t* rgba);

//Return rgba code in rgba[4]
uint8_t* wf3d_color_GetRGBA(wf3d_color const* color, uint8_t* rgba);

//Mix several colors
wf3d_color* wf3d_color_mix_colors(wf3d_color* mixed_color, wf3d_color const* color_list, float const* coeff, int nb_colors);

//filter = float[4]
wf3d_color* wf3d_color_filter(wf3d_color* dst, wf3d_color const* src, float* filter);

typedef struct
{
    uint8_t rgba[4];

} wf3d_color_uint8;

static inline wf3d_color_uint8* wf3d_color_uint8_from_color(wf3d_color_uint8* color8, wf3d_color const* color)
{
    __m128i color_vect = _mm_cvtps_epi32( _mm_mul_ps(_mm_loadu_ps(color->rgba), _mm_set1_ps(255.0)) );
    color_vect = _mm_packus_epi32(color_vect, color_vect);
    color_vect = _mm_packus_epi16(color_vect, color_vect);

    int32_t color_i = _mm_cvtsi128_si32(color_vect);

    for(int k = 0 ; k < 4 ; k++)
    {
        color8->rgba[k] = (uint8_t)(color_i & 0xff);
        color_i >>= 8;
    }

    return color8;
}

static inline wf3d_color* wf3d_color_from_color_uint8(wf3d_color* color, wf3d_color_uint8 const* color8)
{
    int32_t color_i = (color8->rgba[3] << 24) | (color8->rgba[2] << 16) | (color8->rgba[1] << 8) | (color8->rgba[0] << 0);
    __m128i color_vect = _mm_cvtsi32_si128(color_i);
    color_vect = _mm_cvtepu8_epi32(color_vect);

    _mm_storeu_ps(
                    color->rgba,
                    _mm_mul_ps(_mm_cvtepi32_ps(color_vect), _mm_set1_ps(1.0 / 255.0))
                  );

    return color;
}

#endif // WF3D_COLOR_H_INCLUDED
