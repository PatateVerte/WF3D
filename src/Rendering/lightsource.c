#include <WF3D/Rendering/lightsource.h>

#include <string.h>
#include <math.h>

wf3d_lightsource* wf3d_lightsource_transform(wf3d_lightsource* lightsource_dst, wf3d_lightsource const* lightsource_src, owl_v3f32 v, owl_q32 q_rot)
{
    lightsource_dst->type = lightsource_src->type;
    lightsource_dst->color = lightsource_src->color;

    switch(lightsource_src->type)
    {
    case WF3D_LIGHTSOURCE_AMBIENT:
        lightsource_dst->ambient_src.intensity = lightsource_src->ambient_src.intensity;
        break;
    case WF3D_LIGHTSOURCE_DIRECTIONAL:
        lightsource_dst->directional_src.intensity = lightsource_src->directional_src.intensity;
        lightsource_dst->directional_src.v_dir = owl_q32_transform_v3f32(q_rot, lightsource_src->directional_src.v_dir);
        break;
    case WF3D_LIGHTSOURCE_PUNCTUAL:
        lightsource_dst->punctual_src.intensity_per_surface = lightsource_src->punctual_src.intensity_per_surface;
        lightsource_dst->punctual_src.v_pos = owl_v3f32_add(
                                                                v,
                                                                owl_q32_transform_v3f32(q_rot, lightsource_src->punctual_src.v_pos)
                                                            );
        break;
    }

    return lightsource_dst;
}

wf3d_color* wf3d_lightsource_enlight_surface(wf3d_lightsource const* lightsource_list, unsigned int nb_lightsources, wf3d_color* color_ret, wf3d_surface const* surface, owl_v3f32 v_pos, owl_v3f32 normal)
{
    __m128 final_color_rgba = _mm_setzero_ps();
    __m128 surface_diffusion_color_rgba = _mm_loadu_ps(surface->diffusion_color.rgba);

    for(unsigned int k = 0 ; k < nb_lightsources ; k++)
    {
        wf3d_lightsource const* lightsource = lightsource_list + k;
        float intensity = 0.0;

        switch(lightsource->type)
        {
        case WF3D_LIGHTSOURCE_AMBIENT :
            intensity = lightsource->ambient_src.intensity;
            break;
        case WF3D_LIGHTSOURCE_DIRECTIONAL:
            intensity = lightsource->directional_src.intensity * fmaxf(
                                                                        0.0,
                                                                        -owl_v3f32_dot(
                                                                                        normal,
                                                                                        lightsource->directional_src.v_dir
                                                                                      )
                                                                       );
            break;
        case WF3D_LIGHTSOURCE_PUNCTUAL:
            {
                owl_v3f32 v = owl_v3f32_sub(lightsource->punctual_src.v_pos, v_pos);
                float square_norm = owl_v3f32_dot(v, v);
                intensity = lightsource->directional_src.intensity * fmaxf(
                                                                            0.0,
                                                                            owl_v3f32_dot(v, normal) / (square_norm * sqrtf(square_norm))
                                                                            );
            }
            break;
        }

        final_color_rgba = _mm_add_ps(
                                        final_color_rgba,
                                        _mm_mul_ps(
                                                    _mm_mul_ps(_mm_set1_ps(intensity), _mm_loadu_ps(lightsource->color.rgba)),
                                                    surface_diffusion_color_rgba
                                                   )
                                      );
    }

    final_color_rgba = _mm_insert_ps(final_color_rgba, surface_diffusion_color_rgba, 0b11110000);
    _mm_storeu_ps(color_ret->rgba, final_color_rgba);

    return color_ret;
}
