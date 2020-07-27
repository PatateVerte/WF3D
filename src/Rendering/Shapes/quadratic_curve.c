#include <WF3D/Rendering/Shapes/quadratic_curve.h>

#include <math.h>

//Set up  quadratic curve
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
//Optional parameters (modified only if an intersection has been found) :
//t to return the parameter for the nearest intersection (v_intersection = ray_origin + t*ray_dir)
//normal_ret to return the normal of the intersection
//surface_ret to return the surface of the intersection
bool wf3d_quadratic_curve_NearestIntersectionWithRay(wf3d_quadratic_curve const* curve, owl_v3f32 v_pos, owl_q32 q_rot, owl_v3f32 ray_origin, owl_v3f32 ray_dir, float t_min, float t_max, float* t_ret, owl_v3f32* normal_ret)
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
    }

    return intersection_exists;
}

//
//
//
typedef struct
{
    wf3d_camera3d const* cam;

    wf3d_quadratic_curve const* curve;
    owl_v3f32 v_pos;
    owl_q32 q_rot;

    wf3d_rasterization_callback const* curve_callback;

} wf3d_rasterization_triangle3d_callback_arg;

//
//
//
static void wf3d_rasterization_triangle3d_callback(wf3d_rasterization_rectangle const* rect, int x, int y, void const* callback_arg, owl_v3f32 v_intersection, owl_v3f32 normal)
{
    (void)normal;

    wf3d_rasterization_triangle3d_callback_arg const* arg = callback_arg;
    wf3d_camera3d const* cam = arg->cam;
    wf3d_rasterization_callback const* curve_callback = arg->curve_callback;

    float t_min = - cam->near_clipping_distance / owl_v3f32_unsafe_get_component(v_intersection, 2);
    float t;
    owl_v3f32 curve_normal;
    if(wf3d_quadratic_curve_NearestIntersectionWithRay(arg->curve, arg->v_pos, arg->q_rot, owl_v3f32_zero(), v_intersection, t_min, INFINITY, &t, &curve_normal))
    {
        if(!cam->blackface_culling_enabled || owl_v3f32_dot(curve_normal, v_intersection) <= 0.0)
        {
            curve_callback->callback_fct(rect, x, y, curve_callback->callback_arg, owl_v3f32_scalar_mul(v_intersection, t), curve_normal);
        }
    }
}

//
//
//
void wf3d_quadratic_curve_Rasterization(wf3d_quadratic_curve const* curve, wf3d_rasterization_callback const* callback, wf3d_rasterization_rectangle const* rect, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam)
{
    owl_v3f32 inv_vect_eigenbasis = owl_v3f32_comp_div(owl_v3f32_broadcast(1.0), curve->norminf_filter);
    bool optimized_rasterization = (isfinite(owl_v3f32_norm(inv_vect_eigenbasis)) != 0);

    if(optimized_rasterization)
    {
        owl_q32 q_rot_eigenbasis = owl_q32_mul(q_rot, curve->q_eigenbasis);

        owl_v3f32 adapted_base[3];
        owl_v3f32_setbase_xyz(
                                adapted_base,
                                owl_v3f32_unsafe_get_component(inv_vect_eigenbasis, 0),
                                owl_v3f32_unsafe_get_component(inv_vect_eigenbasis, 1),
                                owl_v3f32_unsafe_get_component(inv_vect_eigenbasis, 2)
                              );
        for(unsigned int bk = 0 ; bk < 3 ; bk++)
        {
            adapted_base[bk] = owl_q32_transform_v3f32(q_rot_eigenbasis, adapted_base[bk]);
        }

        //Usual sign convention
        int vertex_is_clipped = 0;
        for(float sign_z = -1.0 ; sign_z <= 1.0 ; sign_z += 2.0)
        {
            owl_v3f32 vertex_z = owl_v3f32_add_scalar_mul(v_pos, adapted_base[2], sign_z);
            for(float sign_y = -1.0 ; sign_y <= 1.0 ; sign_y += 2.0)
            {
                owl_v3f32 vertex_yz = owl_v3f32_add_scalar_mul(vertex_z, adapted_base[1], sign_y);
                for(float sign_x = -1.0 ; sign_x <= 1.0 ; sign_x += 2.0)
                {
                    owl_v3f32 vertex_xyz = owl_v3f32_add_scalar_mul(vertex_yz, adapted_base[0], sign_x);
                    float z_vertex = owl_v3f32_unsafe_get_component(vertex_xyz, 2);
                    if(z_vertex > - cam->near_clipping_distance)
                    {
                        vertex_is_clipped |= 1 << (2 * ((int)sign_z + 1) + ((int)sign_y + 1) + ((int)sign_x + 1) / 2);
                    }
                }
            }
        }

        //Rasterization process if at least one vertex is not behind the camera
        if(vertex_is_clipped != 0xff)
        {
            wf3d_camera3d custom_cam = *cam;

            wf3d_rasterization_triangle3d_callback_arg triangle_callback_arg;
            triangle_callback_arg.cam = cam;
            triangle_callback_arg.curve = curve;
            triangle_callback_arg.q_rot = q_rot;
            triangle_callback_arg.v_pos = v_pos;
            triangle_callback_arg.curve_callback = callback;
            custom_cam.blackface_culling_enabled = (vertex_is_clipped == 0x00);

            wf3d_rasterization_callback triangle_callback;
            triangle_callback.callback_arg = (void*)&triangle_callback_arg;
            triangle_callback.callback_fct = &wf3d_rasterization_triangle3d_callback;

            for(unsigned int bk0 = 0 ; bk0 < 3 ; bk0++)
            {
                unsigned int bk1 = (bk0 + 1) % 3;
                unsigned int bk2 = (bk0 + 2) % 3;

                owl_v3f32 positive_face_normal = owl_v3f32_normalize(adapted_base[bk0]);

                for(float face_sign = -1.0 ; face_sign <= 1.0 ; face_sign += 2.0)
                {
                    owl_v3f32 face_center = owl_v3f32_add_scalar_mul(v_pos, adapted_base[bk0], face_sign);

                    wf3d_triangle3d triangle;
                    triangle.normal = owl_v3f32_scalar_mul(positive_face_normal, face_sign);
                    triangle.vertex_list[0] = owl_v3f32_add(
                                                                face_center,
                                                                owl_v3f32_add(adapted_base[bk1], adapted_base[bk2])
                                                            );
                    triangle.vertex_list[1] = owl_v3f32_sub(
                                                                face_center,
                                                                owl_v3f32_add(adapted_base[bk1], adapted_base[bk2])
                                                            );

                    for(float corner_sign = -1.0 ; corner_sign <= 1.0 ; corner_sign += 2.0)
                    {
                        triangle.vertex_list[2] = owl_v3f32_add_scalar_mul(
                                                                            face_center,
                                                                            owl_v3f32_sub(adapted_base[bk1], adapted_base[bk2]),
                                                                            corner_sign
                                                                           );
                        wf3d_triangle3d_Rasterization(&triangle, &triangle_callback, rect, owl_v3f32_zero(), owl_q32_from_real(1.0), &custom_cam);
                    }
                }
            }
        }
    }
    else
    {
        float half_height = 0.5 * (float)rect->height;
        float half_width = 0.5 * (float)rect->width;

        for(int y = rect->y_min ; y < rect->y_max ; y++)
        {
            float y_f = ((float)y - half_height + 0.5) * (cam->tan_v_half_opening_angle / half_height);

            for(int x = rect->x_min ; x < rect->x_max ; x++)
            {
                float x_f = ((float)x - half_width + 0.5) * (cam->tan_h_half_opening_angle / half_width);

                owl_v3f32 dir_vect = owl_v3f32_set(x_f, y_f, -1.0);
                float t;
                owl_v3f32 normal;
                if(wf3d_quadratic_curve_NearestIntersectionWithRay(curve, v_pos, q_rot, owl_v3f32_zero(), dir_vect, cam->near_clipping_distance, INFINITY, &t, &normal))
                {
                    callback->callback_fct(rect, x, y, callback->callback_arg, owl_v3f32_scalar_mul(dir_vect, t), normal);
                }
            }
        }
    }
}
