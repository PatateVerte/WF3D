#include <WF3D/Rendering/Design/color.h>

#include <string.h>

#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>

wf3d_color* wf3d_color_mix(wf3d_color* mixed_color, wf3d_color const* const* color_list, float const* coeff, unsigned int nb_colors)
{
    __m128 acc_f = _mm_setzero_ps();

    for(int k = 0 ; k < nb_colors ; k++)
    {
        acc_f = _mm_add_ps(
                            acc_f,
                            _mm_mul_ps( _mm_loadu_ps(color_list[k]->rgba), _mm_set1_ps(coeff[k]) )
                           );

    }

    _mm_storeu_ps(mixed_color->rgba, acc_f);

    return mixed_color;
}

wf3d_color* wf3d_color_mix8(wf3d_color* mixed_color, wf3d_color_uint8 const* const* color_list, float const* coeff, unsigned int nb_colors)
{
    __m128 acc_f = _mm_setzero_ps();

    for(int k = 0 ; k < nb_colors ; k++)
    {
        int32_t color_i = (color_list[k]->rgba[3] << 24) | (color_list[k]->rgba[2] << 16) | (color_list[k]->rgba[1] << 8) | (color_list[k]->rgba[0] << 0);
        __m128i color_vect = _mm_cvtsi32_si128(color_i);
        color_vect = _mm_cvtepu8_epi32(color_vect);
        __m128 color_vect_f = _mm_mul_ps(_mm_cvtepi32_ps(color_vect), _mm_set1_ps(1.0 / 255.0));

        acc_f = _mm_add_ps(
                            acc_f,
                            _mm_mul_ps( color_vect_f, _mm_set1_ps(coeff[k]) )
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
    __m128 src_vect = _mm_loadu_ps(src->rgba);
    __m128 color_vect = _mm_mul_ps(
                                    src_vect,
                                    _mm_loadu_ps(filter)
                                   );
    color_vect = _mm_insert_ps(color_vect, src_vect, 0b11110000);
    _mm_storeu_ps(
                    dst->rgba,
                    color_vect
                  );

    return dst;
}
