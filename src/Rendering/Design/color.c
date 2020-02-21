#include <WF3D/Rendering/Design/color.h>

#include <string.h>

#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>

//Set a color
//
//
wf3d_color* wf3d_color_SetRGB(wf3d_color* color, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    int32_t color_i = (a << 24) | (b << 16) | (g << 8) | (r << 0);
    __m128i color_vect = _mm_cvtsi32_si128(color_i);
    color_vect = _mm_cvtepu8_epi32(color_vect);

    _mm_storeu_ps(
                    color->rgba,
                    _mm_mul_ps(_mm_cvtepi32_ps(color_vect), _mm_set1_ps(1.0 / 255.0))
                  );
    return color;
}

//Return rgba code in rgba[4]
//
//
uint8_t* wf3d_color_GetRGB(wf3d_color const* color, uint8_t* rgba)
{
    __m128i color_vect = _mm_cvtps_epi32( _mm_mul_ps(_mm_loadu_ps(color->rgba), _mm_set1_ps(255.0)) );
    color_vect = _mm_packus_epi32(color_vect, color_vect);
    color_vect = _mm_packus_epi16(color_vect, color_vect);

    int32_t color_i = _mm_cvtsi128_si32(color_vect);

    for(int k = 0 ; k < 4 ; k++)
    {
        rgba[k] = (uint8_t)(color_i & 0xff);
        color_i >>= 8;
    }

    return rgba;
}

wf3d_color* wf3d_color_mix_colors(wf3d_color* mixed_color, wf3d_color const* color_list, float const* coeff, int nb_colors)
{
    __m128 acc_f = _mm_setzero_ps();

    for(int k = 0 ; k < nb_colors ; k++)
    {
        acc_f = _mm_add_ps(
                            acc_f,
                            _mm_mul_ps( _mm_loadu_ps(color_list[k].rgba), _mm_set1_ps(coeff[k]) )
                           );

    }

    _mm_storeu_ps(mixed_color->rgba, acc_f);

    return mixed_color;
}

//filter = float[4]
//
//
wf3d_color* wf3d_color_filter(wf3d_color* dst, wf3d_color const* src, float* filter)
{
    _mm_storeu_ps(
                    dst->rgba,
                    _mm_mul_ps(
                                _mm_loadu_ps(src->rgba),
                                _mm_loadu_ps(filter)
                               )
                  );

    return dst;
}
