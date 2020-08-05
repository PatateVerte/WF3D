#include <WF3D/Rendering/Shapes/triangle3d.h>

//
//
//
wf3d_triangle3d* wf3d_triangle3d_Set(wf3d_triangle3d* triangle, owl_v3f32* vertex_list, owl_v3f32 normal)
{
    for(unsigned int vi = 0 ; vi < 3 ; vi++)
    {
        triangle->vertex_list[vi] = vertex_list[vi];
    }

    triangle->normal = normal;

    return triangle;
}

//
//
//
wf3d_triangle3d* wf3d_triangle3d_ComputeNormal(wf3d_triangle3d* triangle)
{
    owl_v3f32 v1 = triangle->vertex_list[1] - triangle->vertex_list[0];
    owl_v3f32 v2 = triangle->vertex_list[2] - triangle->vertex_list[0];

    triangle->normal = owl_v3f32_normalize(owl_v3f32_cross(v1, v2));

    return triangle;
}

//
//
//
wf3d_triangle3d* wf3d_triangle3d_Move(wf3d_triangle3d* t_dst, wf3d_triangle3d const* t_src, owl_v3f32 v)
{
    for(int i = 0 ; i < 3 ; i++)
    {
        t_dst->vertex_list[i] = owl_v3f32_add(t_dst->vertex_list[i], v);
    }

    t_dst->normal = t_src->normal;

    return t_dst;
}

//
//
//
wf3d_triangle3d* OWL_VECTORCALL wf3d_triangle3d_Transform(wf3d_triangle3d* t_dst, wf3d_triangle3d const* t_src, owl_v3f32 v, owl_q32 q_rot)
{
    for(unsigned int vi = 0 ; vi < 3 ; vi++)
    {
        t_dst->vertex_list[vi] = owl_q32_add(
                                                owl_q32_transform_v3f32(q_rot, t_src->vertex_list[vi]),
                                                v
                                             );
    }
    t_dst->normal = owl_q32_transform_v3f32(q_rot, t_src->normal);

    return t_dst;
}

//The intersection between a ray and the triangle3d
//Return true if the intersection exists and returns the parameter, false otherwise
//Optional parameters (modified only if an intersection has been found) :
//t to return the parameter for the nearest intersection (v_intersection = ray_origin + t*ray_dir)
//normal_ret to return the normal of the intersection
bool OWL_VECTORCALL wf3d_triangle3d_NearestIntersectionWithRay(wf3d_triangle3d const* triangle, owl_v3f32 v_pos, owl_q32 q_rot, owl_v3f32 ray_origin, owl_v3f32 ray_dir, float t_min, float t_max, float* t_ret, owl_v3f32* normal_ret)
{
    bool intersection_found = false;

    owl_q32 q_rot_conj = owl_q32_conj(q_rot);
    owl_v3f32 rel_ray_origin = owl_q32_transform_v3f32(q_rot_conj, owl_v3f32_sub(ray_origin, v_pos));
    owl_v3f32 rel_ray_dir = owl_q32_transform_v3f32(q_rot_conj, ray_dir);
    float t = owl_v3f32_dot(owl_v3f32_sub(triangle->vertex_list[0], rel_ray_origin), triangle->normal) / owl_v3f32_dot(rel_ray_dir, triangle->normal);

    if(t_min <= t && t <= t_max && isfinite(t) != 0)
    {
        owl_v3f32 rel_v_intersection = owl_v3f32_add_scalar_mul(rel_ray_origin, rel_ray_dir, t);

        float det[3] = {0.0, 0.0, 0.0};
        bool same_sign = true;
        for(unsigned int k = 0 ; k < 3 && same_sign; k++)
        {
            unsigned int k1 = (k + 1) % 3;
            unsigned int k2 = (k + 2) % 3;
            det[k] = owl_v3f32_triple(
                                        owl_v3f32_sub(rel_v_intersection, triangle->vertex_list[k1]),
                                        owl_v3f32_sub(triangle->vertex_list[k2], triangle->vertex_list[k1]),
                                        triangle->normal
                                     );
            same_sign = (det[k] * det[k2] >= 0.0f);
        }

        if(same_sign)
        {
            intersection_found = true;

            if(t_ret != NULL)
            {
                *t_ret = t;
            }

            if(normal_ret != NULL)
            {
                *normal_ret = owl_q32_transform_v3f32(q_rot, triangle->normal);
            }
        }
    }

    return intersection_found;
}

//Clipping of a triangle
//triangle_ret_list[2], clipped_callback[2], clipped_callback_arg_buff[2]
//
unsigned int wf3d_triangle3d_Clipping(wf3d_triangle3d* clipped_triangle, wf3d_triangle3d const* triangle, wf3d_camera3d const* cam)
{
    float vertex_coords[3][4] OWL_ALIGN16;

    //Clipping detection
    unsigned int nb_vertices_behind = 0;
    bool vertex_is_behind[3];

    for(unsigned int vi = 0 ; vi < 3 ; vi++)
    {
        //Storing
        owl_v3f32_store4(vertex_coords[vi], triangle->vertex_list[vi]);
        clipped_triangle[0].vertex_list[vi] = triangle->vertex_list[vi];
        float z_vertex = owl_v3f32_unsafe_get_component(triangle->vertex_list[vi], 2);

        //Clipping
        if(z_vertex > - cam->near_clipping_distance)
        {
            vertex_is_behind[vi] = true;
            nb_vertices_behind += 1;
        }
        else
        {
            vertex_is_behind[vi] = false;
        }
    }

    //If there is no need for clipping
    if(nb_vertices_behind == 0)
    {
        clipped_triangle[0].normal = triangle->normal;
        return 1;
    }
    //If there is only one vertex behind
    else if(nb_vertices_behind == 1)
    {
        for(unsigned int p = 0 ; p < 2 ; p++)
        {
            clipped_triangle[p].normal = triangle->normal;
        }

        unsigned int vi_clipped = 0;
        for(unsigned int vi = 0 ; vi < 3 ; vi++)
        {
            if(vertex_is_behind[vi])
            {
                vi_clipped = vi;
            }
        }

        for(unsigned int vk = 0 ; vk < 2 ; vk++)
        {
            unsigned int const vi_minus_one = (vi_clipped + vk) % 3;
            unsigned int const vi = (vi_clipped + 1 + vk) % 3;

            float const t = (vertex_coords[vi_clipped][2] + cam->near_clipping_distance) / (vertex_coords[vi_clipped][2] - vertex_coords[vi][2]);
            owl_v3f32 clipped_vertex = owl_v3f32_add(
                                                        owl_v3f32_scalar_mul(triangle->vertex_list[vi], t),
                                                        owl_v3f32_scalar_mul(triangle->vertex_list[vi_clipped], 1.0f - t)
                                                     );
            clipped_vertex = owl_v3f32_unsafe_set_component(clipped_vertex, 2, -cam->near_clipping_distance);

            for(unsigned int p = 0 ; p <= vk ; p++)
            {
                //Unmoved vertices
                clipped_triangle[p].vertex_list[vi] = triangle->vertex_list[vi];
            }

            for(unsigned int p = vk ; p < 2 ; p++)
            {
                //Clipped vertices
                clipped_triangle[p].vertex_list[vi_minus_one] = clipped_vertex;
            }

        }

        return 2;
    }
    //If there are 2 vertices behind
    else if(nb_vertices_behind == 2)
    {
        clipped_triangle[0].normal = triangle->normal;

        unsigned int vi_unclipped = 0;
        for(unsigned int vi = 0 ; vi < 3 ; vi++)
        {
            if(!vertex_is_behind[vi])
            {
                vi_unclipped = vi;
            }
        }

        for(unsigned int vi = 0 ; vi < 3 ; vi++)
        {
            if(vertex_is_behind[vi])
            {
                float const t = - (vertex_coords[vi_unclipped][2] + cam->near_clipping_distance) / (vertex_coords[vi][2] - vertex_coords[vi_unclipped][2]);
                owl_v3f32 clipped_vertex = owl_v3f32_add(
                                                                owl_v3f32_scalar_mul(triangle->vertex_list[vi], t),
                                                                owl_v3f32_scalar_mul(triangle->vertex_list[vi_unclipped], 1.0f - t)
                                                            );
                clipped_triangle[0].vertex_list[vi] = owl_v3f32_unsafe_set_component(clipped_vertex, 2, -cam->near_clipping_distance);
            }
            else
            {
                clipped_triangle[0].vertex_list[vi] = triangle->vertex_list[vi];
            }
        }
        return 1;
    }
    else
    {
        return 0;
    }
}

//Rasterization with custom callback function
//
//
void wf3d_triangle3d_RasterizationAfterClipping(wf3d_triangle3d const* triangle, wf3d_rasterization_rectangle const* rect, wf3d_rasterization_callback const* callback, wf3d_camera3d const* cam)
{
    float half_width = 0.5f * (float)rect->width;
    float half_height = 0.5f * (float)rect->height;

    float const x_scale = half_width / cam->tan_h_half_opening_angle;
    float const y_scale = half_height / cam->tan_v_half_opening_angle;
    float const inv_x_scale = 1.0f / x_scale;
    float const inv_y_scale = 1.0f / y_scale;

    owl_v3f32 rel_vertex[3];
    float vertex_coords[3][4] OWL_ALIGN16;

    for(unsigned int vi = 0 ; vi < 3 ; vi++)
    {
        rel_vertex[vi] = triangle->vertex_list[vi];
        //Storing
        owl_v3f32_store4(vertex_coords[vi], rel_vertex[vi]);
    }

    bool blackface_result = true;
    if(cam->blackface_culling_enabled)
    {
        for(unsigned int vi = 0 ; vi < 3 && blackface_result ; vi++)
        {
            blackface_result = owl_v3f32_dot(rel_vertex[vi], triangle->normal) < 0.0f;
        }
    }

    if(blackface_result)
    {
        float screen_coords[3][2];

        float x_gen_min_f = 2.0f * half_width;
        float x_gen_max_f = 0.0f;
        float y_min_f = 2.0f * half_height;
        float y_max_f = 0.0f;

        for(int i = 0 ; i < 3 ; i++)
        {
            float tmp = -1.0f / vertex_coords[i][2];
            screen_coords[i][0] = vertex_coords[i][0] * tmp * x_scale + half_width;
            screen_coords[i][1] = vertex_coords[i][1] * tmp * y_scale + half_height;

            x_gen_min_f = fminf(x_gen_min_f, screen_coords[i][0]);
            x_gen_max_f = fmaxf(x_gen_max_f, screen_coords[i][0]);
            y_min_f = fminf(y_min_f, screen_coords[i][1]);
            y_max_f = fmaxf(y_max_f, screen_coords[i][1]);
        }

        if(x_gen_min_f < 2.0f * half_width && x_gen_max_f >= 0.0f)
        {
            int y_min = (int)(roundf(fmaxf(y_min_f, 0.0f)));
            if(y_min < rect->y_min)
            {
                y_min = rect->y_min;
            }
            int y_max = (int)(roundf(fminf(y_max_f, 2.0f * half_height)));
            if(y_max > rect->y_max)
            {
                y_max = rect->y_max;
            }


            for(int y = y_min ; y < y_max ; y++)
            {
                float y_f = 0.5f + (float)y;

                float x_min_f = 2.0f * half_width;
                float x_max_f = 0.0f;
                for(unsigned int i0 = 0 ; i0 < 3 ; i0++)
                {
                    unsigned int i1 = (i0 + 1) % 3;
                    float t = (y_f - screen_coords[i1][1]) / (screen_coords[i0][1] - screen_coords[i1][1]);
                    if(0.0f <= t && t <= 1.0f)
                    {
                        float a = t;
                        float b = 1.0f - t;
                        float x_f = a * screen_coords[i0][0] + b * screen_coords[i1][0];

                        x_min_f = fminf(x_min_f, x_f);
                        x_max_f = fmaxf(x_max_f, x_f);
                    }
                }

                int x_min = (int)(roundf(fmaxf(x_min_f, 0.0f)));
                if(x_min < rect->x_min)
                {
                    x_min = rect->x_min;
                }
                int x_max = (int)(roundf(fminf(x_max_f, 2.0f * half_width)));
                if(x_max > rect->x_max)
                {
                    x_max = rect->x_max;
                }

                for(int x = x_min ; x < x_max ; x++)
                {
                    float x_f = 0.5f + (float)x;

                    owl_v3f32 v_intersection;
                    {
                        owl_v3f32 dir_vect = owl_v3f32_set(
                                                            (x_f - half_width) * inv_x_scale,
                                                            (y_f - half_height) * inv_y_scale,
                                                            -1.0f
                                                           );

                        float const t = owl_v3f32_dot(rel_vertex[0], triangle->normal) / owl_v3f32_dot(dir_vect, triangle->normal);
                        v_intersection = owl_v3f32_scalar_mul(dir_vect, t);
                    }

                    callback->callback_fct(rect, x, y, callback->callback_arg, v_intersection, triangle->normal);
                }
            }
        }
    }
}

//Rasterization of a triangle
//
//
void OWL_VECTORCALL wf3d_triangle3d_Rasterization(wf3d_triangle3d const* triangle, wf3d_rasterization_callback const* callback, wf3d_rasterization_rectangle const* rect, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam)
{
    wf3d_triangle3d transformed_triangle;
    wf3d_triangle3d_Transform(&transformed_triangle, triangle, v_pos, q_rot);

    wf3d_triangle3d clipped_triangle[2];

    unsigned int nb_clipped_triangle = wf3d_triangle3d_Clipping(clipped_triangle, &transformed_triangle, cam);

    for(unsigned int k = 0 ; k < nb_clipped_triangle ; k++)
    {
        wf3d_triangle3d_RasterizationAfterClipping(clipped_triangle + k, rect, callback, cam);
    }
}
