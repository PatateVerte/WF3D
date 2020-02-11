#include <WF3D/Rendering/lightsource.h>

#include <string.h>
#include <math.h>

//Advanced set up
//
//
wf3d_lightsource* wf3d_lightsource_set(wf3d_lightsource* ls, wf3d_vect3d v_pos, float const* light_color, float R0, float L_0, float L_inf)
{
    ls->v_pos = v_pos;
    memcpy(ls->light_color, light_color, 4 * sizeof(*light_color));
    ls->R0 = R0;
    ls->L_0 = L_0;
    ls->L_inf = L_inf;

    return ls;
}

//
//
//
float* wf3d_lightsource_getlightfilter(float* filter_ret, wf3d_vect3d M, wf3d_vect3d n, wf3d_lightsource const* ls_list, int nb_ls)
{
    __m128 light_filter_vect = _mm_setzero_ps();

    for(int k = 0 ; k < nb_ls ; k++)
    {
        //From M to ls_list[k].v_pos
        wf3d_vect3d v = wf3d_vect3d_sub(ls_list[k].v_pos, M);
        wf3d_vect3d u = wf3d_vect3d_normalize(v);

        float const R0_2 = ls_list[k].R0 * ls_list[k].R0;
        float L = ls_list[k].L_0 * R0_2 / (R0_2 + wf3d_vect3d_dot(v, v));
        L *= 0.5f * (wf3d_vect3d_dot(u, n) + 1.0f);
        L = (L + ls_list[k].L_inf) / (ls_list[k].L_0 + ls_list[k].L_inf);

        light_filter_vect = _mm_add_ps(
                                        light_filter_vect,
                                        _mm_mul_ps(_mm_set1_ps(L), _mm_load_ps(ls_list[k].light_color))
                                       );
    }

    _mm_storeu_ps(filter_ret, light_filter_vect);

    return filter_ret;
}
