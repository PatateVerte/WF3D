#include <WF3D/Rendering/Design/surface.h>

wf3d_surface* wf3d_surface_mix(wf3d_surface* mixed_surface, wf3d_surface const* const* surface_list, float const* coeff, int nb_colors)
{
    __m128 acc_color = _mm_setzero_ps();
    __m128 acc_surface = _mm_setzero_ps();

    for(int k = 0 ; k < nb_colors ; k++)
    {
        acc_color = _mm_add_ps(
                            acc_color,
                            _mm_mul_ps( _mm_loadu_ps(surface_list[k]->color.rgba), _mm_set1_ps(coeff[k]) )
                           );
        acc_surface = _mm_add_ps(
                            acc_surface,
                            _mm_mul_ps( _mm_loadu_ps(surface_list[k]->raw_surface_data), _mm_set1_ps(coeff[k]) )
                           );

    }

    _mm_storeu_ps(mixed_surface->color.rgba, acc_color);
    _mm_storeu_ps(mixed_surface->raw_surface_data, acc_surface);

    return mixed_surface;
}

