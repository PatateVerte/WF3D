#include <WF3D/Rendering/Design/color.h>

#include <string.h>

#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>

wf3d_color* wf3d_color_mix_colors(wf3d_color* mixed_color, wf3d_color const* color_list, float const* coeff, int nb_colors)
{
    __m128 acc_f = _mm_setzero_ps();

    for(int k = 0 ; k < nb_colors ; k++)
    {
        int32_t tmp = 0;
        memcpy(&tmp, color_list[k].data, 4 * sizeof(uint8_t));

        __m128i color_vect = _mm_cvtepu8_epi32(
                                                _mm_cvtsi32_si128(tmp)
                                               );

        __m128 color_vect_f = _mm_cvtepi32_ps(color_vect);
        __m128 coeff_vect = _mm_set1_ps(coeff[k]);

        acc_f = _mm_add_ps(acc_f, _mm_mul_ps(color_vect_f, coeff_vect));
    }

    __m128i acc = _mm_cvtps_epi32(acc_f);
    acc = _mm_packus_epi32(acc, acc);
    acc = _mm_packus_epi16(acc, acc);
    int32_t color_data = _mm_cvtsi128_si32(acc);

    memcpy(mixed_color->data, &color_data, 4 * sizeof(uint8_t));

    return mixed_color;
}

//filter = float[4]
//
//
wf3d_color* wf3d_color_filter(wf3d_color* dst, wf3d_color const* src, float* filter)
{
    int32_t tmp = 0;
    memcpy(&tmp, src->data, 4 * sizeof(uint8_t));

    __m128i color_vect = _mm_cvtepu8_epi32(
                                            _mm_set1_epi32(tmp)
                                           );
    __m128 color_vect_f = _mm_mul_ps(_mm_loadu_ps(filter), _mm_cvtepi32_ps(color_vect));

    color_vect = _mm_cvtps_epi32(color_vect_f);
    color_vect = _mm_packus_epi32(color_vect, color_vect);
    color_vect = _mm_packus_epi16(color_vect, color_vect);
    int32_t color_data = _mm_cvtsi128_si32(color_vect);

    memcpy(dst->data, &color_data, 4 * sizeof(uint8_t));

    return dst;
}
