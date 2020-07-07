#include <WF3D/Rendering/Design/surface.h>

wf3d_surface* wf3d_surface_mix(wf3d_surface* mixed_surface, wf3d_surface const* const* surface_list, float const* coeff, int nb_colors)
{
    __m128 acc_color = _mm_setzero_ps();
    __m128 acc_reflection = _mm_setzero_ps();
    __m128 acc_refraction = _mm_setzero_ps();

    for(int k = 0 ; k < nb_colors ; k++)
    {
        acc_color = _mm_add_ps(
                            acc_color,
                            _mm_mul_ps( _mm_loadu_ps(surface_list[k]->diffusion_color.rgba), _mm_set1_ps(coeff[k]) )
                           );
        acc_reflection = _mm_add_ps(
                                        acc_reflection,
                                        _mm_mul_ps( _mm_loadu_ps(surface_list[k]->reflection_filter), _mm_set1_ps(coeff[k]) )
                                    );
        acc_refraction = _mm_add_ps(
                                        acc_refraction,
                                        _mm_mul_ps( _mm_loadu_ps(surface_list[k]->refraction_filter), _mm_set1_ps(coeff[k]) )
                                    );

    }

    _mm_storeu_ps(mixed_surface->diffusion_color.rgba, acc_color);
    _mm_storeu_ps(mixed_surface->reflection_filter, acc_reflection);
    _mm_storeu_ps(mixed_surface->refraction_filter, acc_refraction);

    return mixed_surface;
}

