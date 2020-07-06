#include <WF3D/Rendering/Shapes/quadratic_curve.h>

#include <math.h>

//
//
//
wf3d_quadratic_curve* wf3d_quadratic_curve_set(wf3d_quadratic_curve* curve, owl_q32 q_eigenbasis, owl_v3f32 norminf_filter, owl_v3f32 norm2_filter, float c, owl_v3f32 a, owl_v3f32 alpha, wf3d_surface const* surface_data)
{
    curve->q_eigenbasis = q_eigenbasis;

    curve->norminf_filter = norminf_filter;
    curve->norm2_filter = norm2_filter;

    curve->c = c;
    curve->a = a;
    curve->alpha = alpha;

    curve->surface_data = *surface_data;

    return curve;
}

//The intersection between a ray and the quadratic curve
//Return true if the intersection exists and returns the parameter, false otherwise
//t contains the parameter for the nearest intersection
bool wf3d_quadratic_curve_NearestIntersectionWithRay(wf3d_quadratic_curve const* curve, owl_v3f32 v_pos, owl_v3f32 q_rot, owl_v3f32 ray_origin, owl_v3f32 ray_dir, float t_min, float t_max, float* t)
{
    bool intersection_exists = false;
    float t_found = INFINITY;

    owl_q32 q_rot_eigenbasis = owl_q32_mul(q_rot, curve->q_eigenbasis);

    owl_v3f32 rel_ray_origin = owl_q32_transform_v3f32(
                                                    owl_q32_conj(q_rot_eigenbasis),
                                                    owl_v3f32_sub(ray_origin, v_pos)
                                                  );
    owl_v3f32 rel_ray_dir = owl_q32_transform_v3f32(owl_q32_conj(q_rot_eigenbasis), ray_dir);

    float a;
    float b;
    float c;
    {
        a = owl_v3f32_dot(
                            curve->alpha,
                            owl_v3f32_comp_mul(rel_ray_dir, rel_ray_dir)
                         );

        owl_v3f32 alpha_times_origin = owl_v3f32_comp_mul(curve->alpha, rel_ray_origin);

        b = owl_v3f32_dot(
                            rel_ray_dir,
                            owl_v3f32_add_scalar_mul(curve->a, alpha_times_origin, 2.0)
                         );
        c = curve->c + owl_v3f32_dot(
                                        rel_ray_origin,
                                        owl_v3f32_add(curve->a, alpha_times_origin)
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
                if(t_min <= t[i] && t[i] <= t_max)
                {
                    owl_v3f32 rel_Pi = owl_v3f32_add_scalar_mul(rel_ray_origin, rel_ray_dir, t[i]);
                    owl_v3f32 v_inf = owl_v3f32_comp_mul(rel_Pi, curve->norminf_filter);
                    owl_v3f32 v_2 = owl_v3f32_comp_mul(rel_Pi, curve->norm2_filter);

                    if(owl_v3f32_norminf(v_inf) <= 1.0 && owl_v3f32_dot(v_2, v_2) <= 1.0)
                    {
                        t_found = fminf(t_found, t[i]);
                        intersection_exists = true;
                    }
                }
            }
        }
    }
    else if(b != 0.0)
    {
        float t1 = - c / b;
        if(t_min <= t1 && t1 <= t_max)
        {
            owl_v3f32 rel_P1 = owl_v3f32_add_scalar_mul(rel_ray_origin, rel_ray_dir, t1);
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

    float half_height = 0.5 * (float)img_out->height;
    float half_width = 0.5 * (float)img_out->width;

    owl_q32 q_rot_eigenbasis = owl_q32_mul(q_rot, curve->q_eigenbasis);

    wf3d_color mix_with_black_color_array[2] ={(wf3d_color){.rgba = {0.0, 0.0, 0.0, 1.0}}};

    for(int y = 0 ; y < img_out->height && error == WF3D_SUCCESS ; y++)
    {
        float y_f = ((float)y - half_height) * (cam->tan_v_half_opening_angle / half_height);

        for(int x = 0 ; x < img_out->width && error == WF3D_SUCCESS; x++)
        {
            float x_f = ((float)x - half_width) * (cam->tan_h_half_opening_angle / half_width);

            owl_v3f32 v_dir = owl_v3f32_set(x_f, y_f, -1.0);
            float t_min = (1.0 + x_f*x_f + y_f*y_f) * cam->near_clipping_distance;
            float t_max = (1.0 + x_f*x_f + y_f*y_f) * cam->far_clipping_distance;
            float t = 0.0;
            if(wf3d_quadratic_curve_NearestIntersectionWithRay(curve, v_pos, q_rot, owl_v3f32_zero(), v_dir, t_min, t_max, &t))
            {
                owl_v3f32 v_intersection = owl_v3f32_scalar_mul(v_dir, t);
                float depth = - owl_v3f32_unsafe_get_component(v_intersection, 2);
                if(depth <= cam->far_clipping_distance && depth <= wf3d_Image2d_unsafe_Depth(img_out, x, y))
                {
                    owl_v3f32 v_intersection_eigenbasis = owl_q32_transform_v3f32(
                                                                                    owl_q32_conj(q_rot_eigenbasis),
                                                                                    owl_v3f32_sub(v_intersection, v_pos)
                                                                                  );
                    owl_v3f32 normal_eigenbasis = owl_v3f32_add_scalar_mul(
                                                                            curve->a,
                                                                            owl_v3f32_comp_mul(curve->alpha, v_intersection_eigenbasis),
                                                                            2.0
                                                                           );
                    owl_v3f32 normal = owl_v3f32_normalize(owl_q32_transform_v3f32(q_rot_eigenbasis, normal_eigenbasis));

                    float mix_with_black_diffusion[2] = {1.0 - curve->surface_data.diffusion, curve->surface_data.diffusion};
                    mix_with_black_color_array[1] = curve->surface_data.color;
                    wf3d_color surface_color;
                    wf3d_color_mix(&surface_color, mix_with_black_color_array, mix_with_black_diffusion, 2);

                    wf3d_color final_color = {.rgba = {0.0, 0.0, 0.0, 0.0}};
                    if(nb_lightsources == 0)
                    {
                        final_color = surface_color;
                    }
                    else
                    {
                        for( unsigned int k = 0 ; k < nb_lightsources ; k++)
                        {
                            wf3d_color lightsource_color;
                            wf3d_lightsource_enlight(lightsource_list + k, &lightsource_color, &surface_color, v_intersection, normal);

                            for(unsigned int i = 0 ; i < 3 ; i++)
                            {
                                final_color.rgba[i] += lightsource_color.rgba[i];
                            }
                        }
                        final_color.rgba[3] = surface_color.rgba[3];
                    }

                    error = wf3d_Image2d_SetPixel(img_out, x, y, &final_color, depth);
                }
            }
        }
    }

    return error;
}


