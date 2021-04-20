#ifndef WF3D_COLOR_H_INCLUDED
#define WF3D_COLOR_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>

#include <OWL/owl.h>

#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <smmintrin.h>

#include <math.h>

//
//  COLOR F32
//
typedef struct
{
    float OWL_ALIGN16 rgb[4];

} wf3d_color;

//
static inline wf3d_color wf3d_color_set(float r, float g, float b)
{
    wf3d_color ret;
    ret.rgb[0] = r;
    ret.rgb[1] = g;
    ret.rgb[2] = b;
    ret.rgb[3] = 0.0;
    return ret;
}

//
static inline wf3d_color wf3d_color_black()
{
    return (wf3d_color){.rgb = {0.0, 0.0, 0.0, 0.0}};
}

//
OWL_DLL_EXPORT bool wf3d_color_isblack(wf3d_color color);

//
static inline wf3d_color wf3d_color_scalar_mul(wf3d_color color, float f)
{
    wf3d_color res_color = wf3d_color_black();
    for(unsigned int k = 0 ; k < 4 ; k++)
    {
        res_color.rgb[k] = f * color.rgb[k];
    }
    return res_color;
}

//
static inline wf3d_color wf3d_color_add(wf3d_color color1, wf3d_color color2)
{
    wf3d_color res_color = wf3d_color_black();
    for(unsigned int k = 0 ; k < 4 ; k++)
    {
        res_color.rgb[k] = color1.rgb[k] + color2.rgb[k];
    }
    return res_color;
}

//
static inline wf3d_color wf3d_color_mul(wf3d_color color1, wf3d_color color2)
{
    wf3d_color res_color = wf3d_color_black();
    for(unsigned int k = 0 ; k < 4 ; k++)
    {
        res_color.rgb[k] = color1.rgb[k] * color2.rgb[k];
    }
    return res_color;
}

//
static inline wf3d_color wf3d_color_add_scalar_mul(wf3d_color color1, wf3d_color color2, float f)
{
    wf3d_color res_color = wf3d_color_black();
    for(unsigned int k = 0 ; k < 4 ; k++)
    {
        res_color.rgb[k] = color1.rgb[k] + f * color2.rgb[k];
    }
    return res_color;
}

//Mix several colors
static inline wf3d_color wf3d_color_mix(wf3d_color const* color_list, float const* coeff, unsigned int nb_colors)
{
    wf3d_color mixed_color = wf3d_color_black();
    for(unsigned int k = 0 ; k < nb_colors ; k++)
    {
        mixed_color = wf3d_color_add_scalar_mul(mixed_color, color_list[k], coeff[k]);
    }
    return mixed_color;
}

//filter = float[4]
static inline wf3d_color wf3d_color_filter(wf3d_color color, float const* filter)
{
    wf3d_color res_color = wf3d_color_black();
    for(unsigned int k = 0 ; k < 4 ; k++)
    {
        res_color.rgb[k] = filter[k] * color.rgb[k];
    }
    return res_color;
}

//
//  COLOR UINT8
//
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
                    _mm_mul_ps(_mm_cvtepi32_ps(color_vect), _mm_set1_ps(1.0f / 255.0f))
                  );

    return color;
}

static inline float wf3d_color_uint8_luminance(wf3d_color_uint8 const* color8)
{
    return (0.3f * (float)color8->rgb[0] + 0.59f * (float)color8->rgb[1] + 0.11f * (float)color8->rgb[2]) / 255.0f;
}

//
//COLOR F16
//
typedef struct
{
    int64_t data;

} wf3d_color_f16;

static inline wf3d_color_f16 wf3d_color_f16_set(float r, float g, float b)
{
    wf3d_color_f16 ret;
    ret.data = _mm_cvtsi128_si64(_mm_cvtps_ph(_mm_set_ps(0.0, b, g, r), 0));
    return ret;
}

static inline wf3d_color_f16 wf3d_color_f16_from_color(wf3d_color color)
{
    wf3d_color_f16 ret;
    ret.data = _mm_cvtsi128_si64(_mm_cvtps_ph(_mm_loadu_ps(color.rgb), 0));
    return ret;
}

static inline wf3d_color wf3d_color_from_color_f16(wf3d_color_f16 color16)
{
    wf3d_color color;
    _mm_storeu_ps(color.rgb, _mm_cvtph_ps(_mm_cvtsi64_si128(color16.data)));
    return color;
}

#endif // WF3D_COLOR_H_INCLUDED
