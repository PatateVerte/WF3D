#include <WF3D/Rendering/lightsource.h>

#include <string.h>

//Advanced set up
//
//
wf3d_lightsource* wf3d_lightsource_set(wf3d_lightsource* ls, wf3d_vect3d v_pos, float const* light_color, float I_0, float I_min, float I_max)
{
    ls->v_pos = v_pos;
    memcpy(ls->light_color, light_color, 4 * sizeof(*light_color));
    ls->I_0 = I_0;
    ls->I_min = I_min;
    ls->I_max = I_max;

    return ls;
}

//Small set up
//
//
wf3d_lightsource* wf3d_lightsource_set1(wf3d_lightsource* ls, wf3d_vect3d v_pos, float I_0)
{
    float const white[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    return wf3d_lightsource_set(ls, v_pos, white, I_0, 0.0, 1.0f);
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

        float I = ls_list[k].I_0 * wf3d_vect3d_dot(v, n) / wf3d_vect3d_dot(v, v);
        I = fmaxf(I, ls_list[k].I_min);
        I = fminf(I, ls_list[k].I_max);

        light_filter_vect = _mm_add_ps(
                                        light_filter_vect,
                                        _mm_mul_ps(_mm_set1_ps(I), _mm_load_ps(ls_list[k].light_color))
                                       );
    }

    _mm_storeu_ps(filter_ret, light_filter_vect);

    return filter_ret;
}
