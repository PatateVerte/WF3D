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

//Enlights a surface with a list of lightsources
//||normal|| = ||vision_ray_dir|| = 1
wf3d_color* wf3d_lightsource_enlight_surface(wf3d_lightsource const* const* lightsource_list, unsigned int nb_lightsources, wf3d_color* color_ret, wf3d_surface const* surface, wf3d_color const* diffusion_color, owl_v3f32 v_pos, owl_v3f32 normal, owl_v3f32 vision_ray_dir)
{
    __m128 final_color_rgb = _mm_setzero_ps();

    switch(surface->shading_model)
    {
    case WF3D_PHONG_SHADING:
        {
            for(unsigned int k = 0 ; k < nb_lightsources ; k++)
            {
                wf3d_lightsource const* lightsource = lightsource_list[k];
                float diffusion_intensity = 0.0;
                float specular_intensity = 0.0;

                switch(lightsource->type)
                {
                case WF3D_LIGHTSOURCE_AMBIENT :
                    diffusion_intensity = lightsource->ambient_src.intensity;
                    break;
                case WF3D_LIGHTSOURCE_DIRECTIONAL:
                    {
                        float source_intensity = lightsource->directional_src.intensity;
                        diffusion_intensity = source_intensity * fmaxf(
                                                                        0.0,
                                                                        -owl_v3f32_dot(normal, lightsource->directional_src.v_dir)
                                                                      );
                        owl_v3f32 r = owl_v3f32_add_scalar_mul(
                                                                lightsource->directional_src.v_dir,
                                                                normal,
                                                                -2.0 * owl_v3f32_dot(lightsource->directional_src.v_dir, normal)
                                                               );
                        float specular_factor = fmaxf(0.0, -owl_v3f32_dot(r, vision_ray_dir));
                        specular_intensity = source_intensity * powf(specular_factor, surface->shininess);
                    }
                    break;
                case WF3D_LIGHTSOURCE_PUNCTUAL:
                    {
                        owl_v3f32 v = owl_v3f32_sub(lightsource->punctual_src.v_pos, v_pos);
                        float square_norm = owl_v3f32_dot(v, v);
                        float source_intensity = lightsource->punctual_src.intensity_per_surface / square_norm;
                        diffusion_intensity = source_intensity * fmaxf(0.0, owl_v3f32_dot(v, normal) / sqrtf(square_norm));
                        owl_v3f32 r = owl_v3f32_add_scalar_mul(v, normal, -2.0 * owl_v3f32_dot(v, normal));
                        float specular_factor = fmaxf(0.0, owl_v3f32_dot(r, vision_ray_dir) / owl_v3f32_norm(r));
                        specular_intensity = source_intensity * powf(specular_factor, surface->shininess);
                    }
                    break;
                }

                final_color_rgb = _mm_add_ps(
                                                final_color_rgb,
                                                _mm_mul_ps(
                                                            _mm_mul_ps(_mm_set1_ps(diffusion_intensity), _mm_loadu_ps(lightsource->color.rgb)),
                                                            _mm_loadu_ps(diffusion_color->rgb)
                                                           )
                                              );
                final_color_rgb = _mm_add_ps(
                                                final_color_rgb,
                                                _mm_mul_ps(
                                                            _mm_mul_ps(_mm_set1_ps(specular_intensity), _mm_loadu_ps(lightsource->color.rgb)),
                                                           _mm_loadu_ps(surface->specular_filter)
                                                           )
                                              );
            }
        }
        break;
    default:
        {
            for(unsigned int k = 0 ; k < nb_lightsources ; k++)
            {
                wf3d_lightsource const* lightsource = lightsource_list[k];
                float diffusion_intensity = 0.0;

                switch(lightsource->type)
                {
                case WF3D_LIGHTSOURCE_AMBIENT :
                    diffusion_intensity = lightsource->ambient_src.intensity;
                    break;
                case WF3D_LIGHTSOURCE_DIRECTIONAL:
                    diffusion_intensity = lightsource->directional_src.intensity * fmaxf(
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
                        diffusion_intensity = lightsource->punctual_src.intensity_per_surface * fmaxf(
                                                                                                        0.0,
                                                                                                        owl_v3f32_dot(v, normal) / (square_norm * sqrtf(square_norm))
                                                                                                     );
                    }
                    break;
                }

                final_color_rgb = _mm_add_ps(
                                                final_color_rgb,
                                                _mm_mul_ps(
                                                            _mm_mul_ps(_mm_set1_ps(diffusion_intensity), _mm_loadu_ps(lightsource->color.rgb)),
                                                            _mm_loadu_ps(diffusion_color->rgb)
                                                           )
                                              );
            }
        }
        break;
    }

    final_color_rgb = _mm_insert_ps(final_color_rgb, _mm_loadu_ps(diffusion_color->rgb), 0b11110000);
    _mm_storeu_ps(color_ret->rgb, final_color_rgb);

    return color_ret;
}
