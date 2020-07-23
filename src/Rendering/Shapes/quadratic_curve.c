#include <WF3D/Rendering/Shapes/quadratic_curve.h>

#include <math.h>

//
//
//
wf3d_quadratic_curve* wf3d_quadratic_curve_set(wf3d_quadratic_curve* curve, owl_q32 q_eigenbasis, owl_v3f32 norminf_filter, owl_v3f32 norm2_filter, float c, owl_v3f32 a, owl_v3f32 alpha, wf3d_surface const* surface_data)
{
    wf3d_quadratic_curve_set_design(curve, surface_data);
    return wf3d_quadratic_curve_set_geometry(curve, q_eigenbasis, norminf_filter, norm2_filter, c, a, alpha);
}

//Set up the geometry of a quadratic curve
wf3d_quadratic_curve* wf3d_quadratic_curve_set_geometry(wf3d_quadratic_curve* curve, owl_q32 q_eigenbasis, owl_v3f32 norminf_filter, owl_v3f32 norm2_filter, float c, owl_v3f32 a, owl_v3f32 alpha)
{
    curve->q_eigenbasis = q_eigenbasis;

    curve->norminf_filter = norminf_filter;
    curve->norm2_filter = norm2_filter;

    curve->c = c;
    curve->a = a;
    curve->alpha = alpha;

    return curve;
}

//Set up the design of a quadratic curve
wf3d_quadratic_curve* wf3d_quadratic_curve_set_design(wf3d_quadratic_curve* curve, wf3d_surface const* surface_data)
{
    curve->surface_data = surface_data;
    return curve;
}

//The intersection between a ray and the quadratic curve
//Return true if the intersection exists and returns the parameter, false otherwise
//Optional parameters (modified only if an intersection has been found) :
//t to return the parameter for the nearest intersection (v_intersection = ray_origin + t*ray_dir)
//normal_ret to return the normal of the intersection
//surface_ret to return the surface of the intersection
bool wf3d_quadratic_curve_NearestIntersectionWithRay(wf3d_quadratic_curve const* curve, owl_v3f32 v_pos, owl_q32 q_rot, owl_v3f32 ray_origin, owl_v3f32 ray_dir, float t_min, float t_max, float* t_ret, owl_v3f32* normal_ret, wf3d_surface* surface_ret)
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
            intersection_exists = (owl_v3f32_norminf(v_inf) <= 1.0 && owl_v3f32_dot(v_2, v_2) <= 1.0);
        }
    }

    if(intersection_exists)
    {
        if(t_ret != NULL)
        {
            *t_ret = t_found;
        }

        if(normal_ret != NULL)
        {
            owl_v3f32 v_intersection_eigenbasis = owl_v3f32_add_scalar_mul(rel_ray_origin, rel_ray_dir, t_found);
            owl_v3f32 normal_eigenbasis = owl_v3f32_add_scalar_mul(
                                                                    curve->a,
                                                                    owl_v3f32_comp_mul(curve->alpha, v_intersection_eigenbasis),
                                                                    2.0
                                                                   );
            *normal_ret = owl_v3f32_normalize(owl_q32_transform_v3f32(q_rot_eigenbasis, normal_eigenbasis));
        }

        if(surface_ret != NULL)
        {
            *surface_ret = *curve->surface_data;
        }
    }

    return intersection_exists;
}

//Raserization of a quadratic curve
//
//
wf3d_error wf3d_quadratic_curve_Rasterization(wf3d_quadratic_curve const* curve, wf3d_image2d_rectangle* img_out, wf3d_rasterization_env const* env, owl_v3f32 v_pos, owl_q32 q_rot)
{
    wf3d_error error = WF3D_SUCCESS;

    if(!curve->surface_data->transparent)
    {
        wf3d_camera3d const* cam = env->cam;

        float half_height = 0.5 * (float)img_out->img2d->height;
        float half_width = 0.5 * (float)img_out->img2d->width;

        owl_q32 q_rot_eigenbasis = owl_q32_mul(q_rot, curve->q_eigenbasis);

        float const x_scale = half_width / cam->tan_h_half_opening_angle;
        float const y_scale = half_height / cam->tan_v_half_opening_angle;

        int y_min = img_out->y_min;
        int y_max = img_out->y_max;
        int x_min = img_out->x_min;
        int x_max = img_out->x_max;

        owl_v3f32 inf_vect_eigenbasis = owl_v3f32_comp_div(owl_v3f32_broadcast(1.0), curve->norminf_filter);
        if(isfinite(owl_v3f32_dot(inf_vect_eigenbasis, inf_vect_eigenbasis)) != 0)
        {
            float min_depth = INFINITY;

            float y_min_f = cam->tan_h_half_opening_angle;
            float y_max_f = 0.0;
            float x_min_f = cam->tan_v_half_opening_angle;
            float x_max_f = 0.0;

            owl_mxf32_3x3 inf_vect_basis;
            owl_mxf32_3x3_diag(&inf_vect_basis, 1.0);
            for(unsigned int j = 0 ; j < 3 ; j++)
            {
                inf_vect_basis.column[j] = owl_q32_transform_v3f32(
                                                                    q_rot_eigenbasis,
                                                                    owl_v3f32_comp_mul(inf_vect_eigenbasis, inf_vect_basis.column[j])
                                                                   );
            }

            for(float sign_x = -1.0 ; sign_x <= 1.0 ; sign_x += 2.0)
            {
                owl_v3f32 v_corner_x = owl_v3f32_add_scalar_mul(v_pos, inf_vect_basis.column[0], sign_x);
                for(float sign_y = -1.0 ; sign_y <= 1.0 ; sign_y += 2.0)
                {
                    owl_v3f32 v_corner_y = owl_v3f32_add_scalar_mul(v_corner_x, inf_vect_basis.column[1], sign_y);
                    for(float sign_z = -1.0 ; sign_z <= 1.0 ; sign_z += 2.0)
                    {
                        owl_v3f32 v_corner = owl_v3f32_add_scalar_mul(v_corner_y, inf_vect_basis.column[2], sign_z);
                        float x_f = 0.0;
                        float y_f = 0.0;
                        float z_f = owl_v3f32_unsafe_get_component(v_corner, 2);

                        min_depth = fminf(min_depth, -z_f);
                        x_f = -owl_v3f32_unsafe_get_component(v_corner, 0) / z_f;
                        y_f = -owl_v3f32_unsafe_get_component(v_corner, 1) / z_f;

                        y_min_f = fminf(y_min_f, y_f);
                        y_max_f = fmaxf(y_max_f, y_f);
                        x_min_f = fminf(x_min_f, x_f);
                        x_max_f = fmaxf(x_max_f, x_f);
                    }
                }
            }

            if(min_depth >= cam->near_clipping_distance)
            {
                y_min = (int)roundf(fmaxf(0.0, y_min_f * y_scale + half_height));
                if(y_min < img_out->y_min)
                {
                    y_min = img_out->y_min;
                }
                y_max = (int)roundf(fminf(2.0 * half_height, y_max_f * y_scale + half_height));
                if(y_max > img_out->y_max)
                {
                    y_max = img_out->y_max;
                }
                x_min = (int)roundf(fmaxf(0.0, x_min_f * x_scale + half_width));
                if(x_min < img_out->x_min)
                {
                    x_min = img_out->x_min;
                }
                x_max = (int)roundf(fminf(2.0 * half_width, x_max_f * x_scale + half_width));
                if(x_max > img_out->x_max)
                {
                    x_max = img_out->x_max;
                }
            }
        }

        for(int y = y_min ; y < y_max && error == WF3D_SUCCESS ; y++)
        {
            float y_f = ((float)y - half_height + 0.5) * (cam->tan_v_half_opening_angle / half_height);

            for(int x = x_min ; x < x_max && error == WF3D_SUCCESS; x++)
            {
                float x_f = ((float)x - half_width + 0.5) * (cam->tan_h_half_opening_angle / half_width);

                owl_v3f32 v_dir = owl_v3f32_set(x_f, y_f, -1.0);
                float t_min = cam->near_clipping_distance;
                float t_max = fminf(cam->far_clipping_distance, wf3d_Image2d_unsafe_Depth(img_out->img2d, x, y));
                float t = 0.0;
                owl_v3f32 normal;
                if(wf3d_quadratic_curve_NearestIntersectionWithRay(curve, v_pos, q_rot, owl_v3f32_zero(), v_dir, t_min, t_max, &t, &normal, NULL))
                {
                    owl_v3f32 v_intersection = owl_v3f32_scalar_mul(v_dir, t);
                    float depth = - owl_v3f32_unsafe_get_component(v_intersection, 2);

                    wf3d_color final_color;
                    wf3d_lightsource_enlight_surface(env->lightsource_list, env->nb_lightsources, &final_color, curve->surface_data, v_intersection, normal, owl_v3f32_normalize(v_dir));

                    error = wf3d_Image2d_SetPixel(img_out->img2d, x, y, &final_color, depth);
                }
            }
        }
    }

    return error;
}

//Rasterization2 of a quadratic curve
//
//
wf3d_error wf3d_quadratic_curve_Rasterization2(wf3d_quadratic_curve const* curve, wf3d_image3d_image_piece* img_out, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam)
{
    wf3d_error error = WF3D_SUCCESS;

    float half_width = 0.5 * (float)img_out->full_img_width;
    float half_height = 0.5 * (float)img_out->full_img_height;

    owl_q32 q_rot_eigenbasis = owl_q32_mul(q_rot, curve->q_eigenbasis);

    float const x_scale = half_width / cam->tan_h_half_opening_angle;
    float const y_scale = half_height / cam->tan_v_half_opening_angle;

    int y_min = img_out->y_min;
    int y_max = img_out->y_max;
    int x_min = img_out->x_min;
    int x_max = img_out->x_max;

    owl_v3f32 inf_vect_eigenbasis = owl_v3f32_comp_div(owl_v3f32_broadcast(1.0), curve->norminf_filter);
    if(isfinite(owl_v3f32_dot(inf_vect_eigenbasis, inf_vect_eigenbasis)) != 0)
    {
        float min_depth = INFINITY;

        float y_min_f = cam->tan_h_half_opening_angle;
        float y_max_f = 0.0;
        float x_min_f = cam->tan_v_half_opening_angle;
        float x_max_f = 0.0;

        owl_mxf32_3x3 inf_vect_basis;
        owl_mxf32_3x3_diag(&inf_vect_basis, 1.0);
        for(unsigned int j = 0 ; j < 3 ; j++)
        {
            inf_vect_basis.column[j] = owl_q32_transform_v3f32(
                                                                q_rot_eigenbasis,
                                                                owl_v3f32_comp_mul(inf_vect_eigenbasis, inf_vect_basis.column[j])
                                                               );
        }

        for(float sign_x = -1.0 ; sign_x <= 1.0 ; sign_x += 2.0)
        {
            owl_v3f32 v_corner_x = owl_v3f32_add_scalar_mul(v_pos, inf_vect_basis.column[0], sign_x);
            for(float sign_y = -1.0 ; sign_y <= 1.0 ; sign_y += 2.0)
            {
                owl_v3f32 v_corner_y = owl_v3f32_add_scalar_mul(v_corner_x, inf_vect_basis.column[1], sign_y);
                for(float sign_z = -1.0 ; sign_z <= 1.0 ; sign_z += 2.0)
                {
                    owl_v3f32 v_corner = owl_v3f32_add_scalar_mul(v_corner_y, inf_vect_basis.column[2], sign_z);
                    float x_f = 0.0;
                    float y_f = 0.0;
                    float z_f = owl_v3f32_unsafe_get_component(v_corner, 2);

                    min_depth = fminf(min_depth, -z_f);
                    x_f = -owl_v3f32_unsafe_get_component(v_corner, 0) / z_f;
                    y_f = -owl_v3f32_unsafe_get_component(v_corner, 1) / z_f;

                    y_min_f = fminf(y_min_f, y_f);
                    y_max_f = fmaxf(y_max_f, y_f);
                    x_min_f = fminf(x_min_f, x_f);
                    x_max_f = fmaxf(x_max_f, x_f);
                }
            }
        }

        if(min_depth >= cam->near_clipping_distance)
        {
            y_min = (int)roundf(fmaxf(0.0, y_min_f * y_scale + half_height));
            if(y_min < img_out->y_min)
            {
                y_min = img_out->y_min;
            }
            y_max = (int)roundf(fminf(2.0 * half_height, y_max_f * y_scale + half_height));
            if(y_max > img_out->y_max)
            {
                y_max = img_out->y_max;
            }
            x_min = (int)roundf(fmaxf(0.0, x_min_f * x_scale + half_width));
            if(x_min < img_out->x_min)
            {
                x_min = img_out->x_min;
            }
            x_max = (int)roundf(fminf(2.0 * half_width, x_max_f * x_scale + half_width));
            if(x_max > img_out->x_max)
            {
                x_max = img_out->x_max;
            }
        }
    }

    for(int y = y_min ; y < y_max && error == WF3D_SUCCESS ; y++)
    {
        float y_f = ((float)y - half_height + 0.5) * (cam->tan_v_half_opening_angle / half_height);

        for(int x = x_min ; x < x_max && error == WF3D_SUCCESS; x++)
        {
            float x_f = ((float)x - half_width + 0.5) * (cam->tan_h_half_opening_angle / half_width);

            owl_v3f32 v_dir = owl_v3f32_set(x_f, y_f, -1.0);
            float t_min = cam->near_clipping_distance;
            float t_max = fminf(cam->far_clipping_distance, wf3d_Image3d_unsafe_Depth(img_out->img3d, x - img_out->x_min, y - img_out->y_min));
            float t = 0.0;
            owl_v3f32 normal;
            if(wf3d_quadratic_curve_NearestIntersectionWithRay(curve, v_pos, q_rot, owl_v3f32_zero(), v_dir, t_min, t_max, &t, &normal, NULL))
            {
                owl_v3f32 v_intersection = owl_v3f32_scalar_mul(v_dir, t);
                float depth = - owl_v3f32_unsafe_get_component(v_intersection, 2);

                error = wf3d_Image3d_SetPixel(img_out->img3d, x - img_out->x_min, y - img_out->y_min, curve->surface_data, depth, v_intersection, normal);
            }
        }
    }

    return error;
}
