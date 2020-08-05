#include <WF3D/Rendering/Design/color.h>

bool wf3d_color_isblack(wf3d_color color)
{
    __m128 color_vec = _mm_loadu_ps(color.rgb);

    float color_max = _mm_cvtss_f32(color_vec);
    float color_min = _mm_cvtss_f32(color_vec);

    color_max = fmaxf(color_max, _mm_cvtss_f32(_mm_insert_ps(color_vec, color_vec, 0x4e)));
    color_min = fminf(color_min, _mm_cvtss_f32(_mm_insert_ps(color_vec, color_vec, 0x4e)));

    color_max = fmaxf(color_max, _mm_cvtss_f32(_mm_insert_ps(color_vec, color_vec, 0x8e)));
    color_min = fminf(color_min, _mm_cvtss_f32(_mm_insert_ps(color_vec, color_vec, 0x8e)));

    return ((color_max == 0.0f) && (color_min == 0.0f));
}
