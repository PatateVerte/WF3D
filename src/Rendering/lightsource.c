#include <WF3D/Rendering/lightsource.h>

#include <string.h>
#include <math.h>

wf3d_color* wf3d_lightsource_enlight(wf3d_lightsource const* lightsource, wf3d_color* dst, wf3d_color const* src, owl_v3f32 v_pos, owl_v3f32 normal)
{
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

    float light_filter[4] OWL_ALIGN16;
    _mm_storeu_ps(
                    light_filter,
                    _mm_mul_ps(
                                _mm_set1_ps(intensity),
                                _mm_loadu_ps(lightsource->color.rgba)
                               )
                 );
    return wf3d_color_filter(dst, src, light_filter);
}
