#include <WF3D/Rendering/Shapes/quadratic_curve.h>

#include <math.h>

//
//
//
wf3d_quadratic_curve* wf3d_quadratic_curve_set(wf3d_quadratic_curve* curve, owl_q32 q_eigenbasis, owl_v3f32 norminf_filter, owl_v3f32 norm2_filter, float c, owl_v3f32 a, owl_v3f32 alpha)
{
    curve->q_eigenbasis = q_eigenbasis;

    curve->norminf_filter = norminf_filter;
    curve->norm2_filter = norm2_filter;

    curve->c = c;
    curve->a = a;
    curve->alpha = alpha;

    return curve;
}

//The intersection between a ray and the quadratic curve
//Return true if the intersection exists and returns the parameter, false otherwise
//t contains the parameter for the nearest intersection
bool wf3d_quadratic_curve_IntersectionWithRay(wf3d_quadratic_curve const* curve, owl_v3f32 v_pos, owl_v3f32 q_rot, owl_v3f32 ray_origin, owl_v3f32 ray_dir, float* t)
{
    bool intersection_exists = false;
    float t_found = 0.0;

    float a;
    float b;
    float c;
    {
        a = owl_v3f32_dot(
                            curve->a,
                            owl_v3f32_comp_mul(ray_dir, ray_dir)
                         );

        owl_v3f32 a_times_origin = owl_v3f32_comp_mul(curve->alpha, ray_origin);

        b = owl_v3f32_dot(
                            ray_dir,
                            owl_v3f32_add_scalar_mul(curve->a, a_times_origin, 2.0)
                         );
        c = curve->c + owl_v3f32_dot(
                                        ray_origin,
                                        owl_v3f32_add(curve->a, a_times_origin)
                                     );
    }

    if(a != 0.0)
    {
        float Delta = b*b - 4*a*c;
        if(Delta >= 0.0)
        {
            float sqrt_Delta = sqrtf(Delta);
            float t[2] =
            {
                (-b + sqrt_Delta) / (2.0 * a),
                (-b - sqrt_Delta) / (2.0 * a)
            };

            for(unsigned i = 0 ; i < 2 ; i++)
            {
                if(t[i] >= 0.0)
                {
                    owl_v3f32 Pi = owl_v3f32_add_scalar_mul(ray_origin, ray_dir, t[i]);
                    owl_v3f32 rel_Pi = owl_q32_transform_v3f32(
                                                                owl_q32_conj(curve->q_eigenbasis),
                                                                owl_v3f32_sub(Pi, v_pos)
                                                               );
                    owl_v3f32 v_inf = owl_v3f32_comp_mul(rel_Pi, curve->norminf_filter);
                    owl_v3f32 v_2 = owl_v3f32_comp_mul(rel_Pi, curve->norm2_filter);

                    if(owl_v3f32_norminf(v_inf) <= 1.0 && owl_v3f32_dot(v_2, v_2) <= 1.0)
                    {
                        t_found = (intersection_exists) ? t[i] : fminf(t[i], t_found);
                        intersection_exists = true;
                    }
                }
            }
        }
    }
    else if(b != 0.0)
    {
        float t1 = - c / b;
        if(t1 >= 0.0)
        {
            owl_v3f32 P1 = owl_v3f32_add_scalar_mul(ray_origin, ray_dir, t1);
            owl_v3f32 rel_P1 = owl_q32_transform_v3f32(
                                                        owl_q32_conj(curve->q_eigenbasis),
                                                        owl_v3f32_sub(P1, v_pos)
                                                       );
            owl_v3f32 v_inf = owl_v3f32_comp_mul(rel_P1, curve->norminf_filter);
            owl_v3f32 v_2 = owl_v3f32_comp_mul(rel_P1, curve->norm2_filter);

            t_found = t1;
            intersection_exists = owl_v3f32_norminf(v_inf) <= 1.0 && owl_v3f32_dot(v_2, v_2) <= 1.0;
        }
    }

    *t = t_found;
    return intersection_exists;
}

//
//
//
wf3d_error wf3d_quadratic_curve_Rasterization(wf3d_quadratic_curve const* curve, wf3d_Image2d* img_out, wf3d_lightsource const* lightsource_list, unsigned int nb_lightsources, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam)
{
    wf3d_error error = WF3D_SUCCESS;



    return error;
}


