#ifndef WF3D_COLOR_H_INCLUDED
#define WF3D_COLOR_H_INCLUDED

#include <stdint.h>

#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>

#include <math.h>

typedef struct
{
    float rgb[4];

} wf3d_color;

//
static inline wf3d_color* wf3d_color_add(wf3d_color* final_color, wf3d_color const* color1, wf3d_color const* color2)
{
    for(unsigned int k = 0 ; k < 4 ; k++)
    {
        final_color->rgb[k] = color1->rgb[k] + color2->rgb[k];
    }

    return final_color;
}

//Mix several colors
wf3d_color* wf3d_color_mix(wf3d_color* mixed_color, wf3d_color const* color_list, float const* coeff, unsigned int nb_colors);

//filter = float[4]
static inline wf3d_color* wf3d_color_filter(wf3d_color* final_color, wf3d_color const* color, float const* filter)
{
    for(unsigned int k = 0 ; k < 4 ; k++)
    {
        final_color->rgb[k] = color->rgb[k] * filter[k];
    }

    return final_color;
}

typedef struct
{
    uint8_t rgb[4];

} wf3d_color_uint8;

static inline wf3d_color_uint8* wf3d_color_uint8_from_color(wf3d_color_uint8* color8, wf3d_color const* color)
{
    __m128 broadcast255 = _mm_set1_ps(255.0);
    __m128i color_vect = _mm_cvtps_epi32( _mm_min_ps(_mm_mul_ps(_mm_loadu_ps(color->rgb), broadcast255), broadcast255) );
    color_vect = _mm_packus_epi32(color_vect, color_vect);
    color_vect = _mm_packus_epi16(color_vect, color_vect);

    int32_t color_i = _mm_cvtsi128_si32(color_vect);

    for(int k = 0 ; k < 4 ; k++)
    {
        color8->rgb[k] = (uint8_t)(color_i & 0xff);
        color_i >>= 8;
    }

    return color8;
}

static inline wf3d_color* wf3d_color_from_color_uint8(wf3d_color* color, wf3d_color_uint8 const* color8)
{
    int32_t color_i = (color8->rgb[3] << 24) | (color8->rgb[2] << 16) | (color8->rgb[1] << 8) | (color8->rgb[0] << 0);
    __m128i color_vect = _mm_cvtsi32_si128(color_i);
    color_vect = _mm_cvtepu8_epi32(color_vect);

    _mm_storeu_ps(
                    color->rgb,
                    _mm_mul_ps(_mm_cvtepi32_ps(color_vect), _mm_set1_ps(1.0 / 255.0))
                  );

    return color;
}

wf3d_color* wf3d_color_mix8(wf3d_color* mixed_color, wf3d_color_uint8 const* color_list, float const* coeff, unsigned int nb_colors);

static inline float wf3d_color_uint8_luminance(wf3d_color_uint8 const* color8)
{
    return (0.3 * (float)color8->rgb[0] + 0.59 * (float)color8->rgb[1] + 0.11 * (float)color8->rgb[2]) / 255.0;
}

#endif // WF3D_COLOR_H_INCLUDED
