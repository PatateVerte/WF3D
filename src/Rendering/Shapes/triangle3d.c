#include <WF3D/Rendering/Shapes/triangle3d.h>

//
//
//
wf3d_triangle3d* wf3d_triangle3d_Set(
                                        wf3d_triangle3d* triangle, wf3d_vect3d* vertex_list, wf3d_vect3d normal,
                                        wf3d_color* (*color_of)(void const*, wf3d_color*, float const*),
                                        void const* design_data
                                     )
{
    if(triangle == NULL)
    {
        return NULL;
    }

    for(int k = 0 ; k < 3 ; k++)
    {
        triangle->vertex_list[k] = vertex_list[k];
    }

    triangle->normal = normal;

    triangle->color_of = color_of;
    triangle->design_data = design_data;

    return triangle;
}

//
//
//
wf3d_triangle3d* wf3d_triangle3d_ComputeNormal(wf3d_triangle3d* triangle)
{
    wf3d_vect3d v1 = triangle->vertex_list[1] - triangle->vertex_list[0];
    wf3d_vect3d v2 = triangle->vertex_list[2] - triangle->vertex_list[0];

    triangle->normal = wf3d_vect3d_normalize(wf3d_vect3d_cross(v1, v2));

    return triangle;
}

//
//
//
wf3d_triangle3d* wf3d_triangle3d_Move(wf3d_triangle3d* t_dst, wf3d_triangle3d const* t_src, wf3d_vect3d v)
{
    for(int i = 0 ; i < 3 ; i++)
    {
        t_dst->vertex_list[i] = wf3d_vect3d_add(t_dst->vertex_list[i], v);
    }

    return wf3d_triangle3d_CopyDesign(t_dst, t_src);
}

//
//
//
wf3d_triangle3d* wf3d_triangle3d_GeometricTransform(wf3d_triangle3d* t_dst, wf3d_triangle3d const* t_src, wf3d_quat q_rot, wf3d_vect3d v)
{
    for(int i = 0 ; i < 3 ; i++)
    {
        t_dst->vertex_list[i] = wf3d_quat_add(
                                                wf3d_quat_transform_vect3d(q_rot, t_src->vertex_list[i]),
                                                v
                                             );
    }
    t_dst->normal = wf3d_quat_transform_vect3d(q_rot, t_src->normal);

    return t_dst;
}

//
//
//
wf3d_triangle3d* wf3d_triangle3d_CopyAndTransform(wf3d_triangle3d* t_dst, wf3d_triangle3d const* t_src, wf3d_quat q_rot, wf3d_vect3d v)
{
    for(int i = 0 ; i < 3 ; i++)
    {
        t_dst->vertex_list[i] = wf3d_quat_add(
                                                wf3d_quat_transform_vect3d(q_rot, t_src->vertex_list[i]),
                                                v
                                             );
    }
    t_dst->normal = wf3d_quat_transform_vect3d(q_rot, t_src->normal);

    return wf3d_triangle3d_CopyDesign(t_dst, t_src);
}

//
//
//
wf3d_error wf3d_triangle3d_Rasterization(wf3d_triangle3d const* triangle, wf3d_Image3d* img_out, wf3d_vect3d v_pos, wf3d_quat q_rot, wf3d_camera3d const* cam)
{
    if(triangle == NULL)
    {
        return WF3D_SUCCESS;
    }

    wf3d_error error = WF3D_SUCCESS;

    float half_width = 0.5 * (float)img_out->width;
    float half_height = 0.5 * (float)img_out->height;

    float const x_scale = half_width / cam->tan_h_half_opening_angle;
    float const y_scale = half_height / cam->tan_v_half_opening_angle;
    float const inv_x_scale = 1.0 / x_scale;
    float const inv_y_scale = 1.0 / y_scale;

    wf3d_vect3d rel_vertex[3];
    float vertex_coords[3][4] __attribute__( (aligned(16)) );

    wf3d_vect3d opp_size_cross_normal[3];
    wf3d_vect3d rel_normal = wf3d_quat_transform_vect3d(q_rot, triangle->normal);

    float y_min_f = 2.0 * half_height;
    float y_max_f = 0.0;
    float x_gen_min_f = 2.0 * half_width;
    float x_gen_max_f = 0.0;

    float triangle_min_depth = INFINITY;

    //Clipping detection
    int nb_vertices_behind = 0;
    bool vertex_is_behind[3];

    for(int vi = 0 ; vi < 3 ; vi++)
    {
        rel_vertex[vi] = wf3d_vect3d_add(
                                        wf3d_quat_transform_vect3d(q_rot, triangle->vertex_list[vi]),
                                        v_pos
                                       );
        //Storing
        wf3d_vect3d_store4(vertex_coords[vi], rel_vertex[vi]);
        float const z_vertex = vertex_coords[vi][3];

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

        //Min depth
        triangle_min_depth = fminf(triangle_min_depth, -z_vertex);
    }

    for(int vi = 0 ; vi < 3 ; vi++)
    {
        int const vi1 = (vi + 1) % 3;
        int const vi2 = (vi + 2) % 3;
        opp_size_cross_normal[vi] = wf3d_vect3d_cross(
                                                        rel_normal,
                                                        wf3d_vect3d_sub(rel_vertex[vi2], rel_vertex[vi1])
                                                      );
    }

    bool blackface_result = true;
    if(cam->blackface_culling_enabled)
    {
        for(int vi = 0 ; vi < 3 && blackface_result ; vi++)
        {
            blackface_result = wf3d_vect3d_dot(rel_vertex[vi], rel_normal) < 0.0;
        }
    }

    if(blackface_result)
    {
        //If there is no need for clipping
        if(nb_vertices_behind == 0)
        {
            float screen_coords[3][2];

            for(int i = 0 ; i < 3 ; i++)
            {
                float tmp = -1.0f / vertex_coords[i][3];
                screen_coords[i][0] = vertex_coords[i][1] * tmp * x_scale + half_width;
                screen_coords[i][1] = vertex_coords[i][2] * tmp * y_scale + half_height;

                y_min_f = fminf(y_min_f, screen_coords[i][1]);
                y_max_f = fmaxf(y_max_f, screen_coords[i][1]);

                x_gen_max_f = fmaxf(x_gen_min_f, screen_coords[i][0]);
                x_gen_min_f = fminf(x_gen_min_f, screen_coords[i][0]);
            }

            int y_min = (int)roundf(fmaxf(y_min_f, 0.0));
            if(y_min < 0)
            {
                y_min = 0;
            }
            int y_max = (int)roundf(fminf(y_max_f, 2.0f * half_height));
            if(y_max > img_out->height - 1)
            {
                y_max = img_out->height - 1;
            }

            if(x_gen_min_f < 2.0f * half_width && x_gen_max_f >= 0.0)
            {
                for(int y = y_min ; y <= y_max && error == WF3D_SUCCESS; y++)
                {
                    float y_f = 0.5f + (float)y;

                    float x_min_f = 2.0f * half_width;
                    float x_max_f = 0.0;
                    for(int i0 = 0 ; i0 < 3 ; i0++)
                    {
                        int i1 = (i0 + 1) % 3;
                        float t = (y_f - screen_coords[i1][1]) / (screen_coords[i0][1] - screen_coords[i1][1]);
                        if(0.0 <= t && t <= 1.0f)
                        {
                            float a = t;
                            float b = 1.0f - t;
                            float x_f = a * screen_coords[i0][0] + b * screen_coords[i1][0];

                            x_min_f = fminf(x_min_f, x_f);
                            x_max_f = fmaxf(x_max_f, x_f);
                        }
                    }

                    int x_min = (int)roundf(fmaxf(x_min_f, 0.0));
                    if(x_min < 0)
                    {
                        x_min = 0;
                    }
                    int x_max = (int)roundf(fminf(x_max_f, 2.0f * half_width));
                    if(x_max > img_out->width - 1)
                    {
                        x_max = img_out->width - 1;
                    }

                    for(int x = x_min ; x <= x_max && error == WF3D_SUCCESS ; x++)
                    {
                        if(triangle_min_depth <= wf3d_Image3d_unsafe_Depth(img_out, x, y))
                        {
                            float x_f = 0.5f + (float)x;

                            wf3d_vect3d v_intersection;
                            {
                                float dir_vect_coords[4] __attribute__( (aligned(16)) );
                                dir_vect_coords[0] = 0.0;
                                dir_vect_coords[1] = (x_f - half_width) * inv_x_scale;
                                dir_vect_coords[2] = (y_f - half_height) * inv_y_scale;
                                dir_vect_coords[3] = -1.0f;

                                wf3d_vect3d dir_vect = wf3d_vect3d_load4(dir_vect_coords);

                                float const t = wf3d_vect3d_dot(rel_vertex[0], rel_normal) / wf3d_vect3d_dot(dir_vect, rel_normal);
                                v_intersection = wf3d_vect3d_scalar_mul(dir_vect, t);
                            }

                            float depth = - wf3d_vect3d_get_component(v_intersection, 2);

                            if(depth <= cam->far_clipping_distance && depth <= wf3d_Image3d_unsafe_Depth(img_out, x, y))
                            {
                                //Gets barycentric coordinates
                                float barycentric_coords[3];
                                float barycentric_sum = 0.0;
                                for(int vi = 0 ; vi < 3 ; vi++)
                                {
                                    int const vi1 = (vi + 1) % 3;
                                    float const tmp = wf3d_vect3d_dot(
                                                                        opp_size_cross_normal[vi],
                                                                        wf3d_vect3d_sub(v_intersection, rel_vertex[vi1])
                                                                     );
                                    barycentric_coords[vi] = tmp;
                                    barycentric_sum += tmp;
                                }

                                float const inv_barycentric_sum = 1.0f / barycentric_sum;
                                for(int vi = 0 ; vi < 3 ; vi++)
                                {
                                    barycentric_coords[vi] *= inv_barycentric_sum;
                                }

                                wf3d_color final_color;
                                triangle->color_of(triangle->design_data, &final_color, barycentric_coords);

                                error = wf3d_Image3d_SetPixel(img_out, x, y, &final_color, depth, v_intersection, triangle->normal);
                            }
                        }
                    }
                }
            }
        }
        //If there is only one vertex behind
        else if(nb_vertices_behind == 1)
        {
            wf3d_triangle3d_clipped_design clipped_design[2];
            wf3d_triangle3d clipped_triangle[2];

            for(int p = 0 ; p < 2 ; p++)
            {
                clipped_design[p].original_color_of = triangle->color_of;
                clipped_design[p].original_design = triangle->design_data;

                clipped_triangle[p].color_of = wf3d_triangle3d_ClippedDesignCallback;
                clipped_triangle[p].design_data = &clipped_design;
                clipped_triangle[p].normal = rel_normal;

                for(int vi = 0 ; vi < 3 ; vi++)
                {
                    for(int k = 0 ; k < 3 ; k++)
                    {
                        clipped_design[p].vertices_barycentric_coords[vi][k] = 0.0;
                    }
                }
            }

            int vi_clipped = 0;
            for(int vi = 0 ; vi < 3 ; vi++)
            {
                if(vertex_is_behind[vi])
                {
                    vi_clipped = vi;
                }
            }

            for(int vk = 0 ; vk < 2 ; vk++)
            {
                int const vi_minus_one = (vi_clipped + vk) % 3;
                int const vi = (vi_clipped + 1 + vk) % 3;

                float const t = (vertex_coords[vi_clipped][3] + cam->near_clipping_distance) / (vertex_coords[vi_clipped][3] - vertex_coords[vi][3]);
                wf3d_vect3d clipped_vertex = wf3d_vect3d_add(
                                                                wf3d_vect3d_scalar_mul(rel_vertex[vi], t),
                                                                wf3d_vect3d_scalar_mul(rel_vertex[vi_clipped], 1.0f - t)
                                                             );
                clipped_vertex = wf3d_vect3d_set_component(clipped_vertex, 2, -cam->near_clipping_distance);

                for(int p = 0 ; p <= vk ; p++)
                {
                    //Unmoved vertices
                    clipped_triangle[p].vertex_list[vi] = rel_vertex[vi];
                    clipped_design[p].vertices_barycentric_coords[vi][vi] = 1.0f;
                }

                for(int p = vk ; p < 2 ; p++)
                {
                    //Clipped vertices
                    clipped_triangle[p].vertex_list[vi_minus_one] = clipped_vertex;
                    clipped_design[p].vertices_barycentric_coords[vi_minus_one][vi] = t;
                    clipped_design[p].vertices_barycentric_coords[vi_minus_one][vi_clipped] = 1.0f - t;
                }

            }

            wf3d_vect3d const clipped_v_pos = wf3d_vect3d_zero();
            wf3d_vect3d const clipped_q_rot = wf3d_quat_from_real(1.0f);
            for(int p = 0 ; p < 2 && error == WF3D_SUCCESS ; p++)
            {
                error = wf3d_triangle3d_Rasterization(clipped_triangle + p, img_out, clipped_v_pos, clipped_q_rot, cam);
            }
        }
        //If there are 2 vertices behind
        else if(nb_vertices_behind == 2)
        {
            wf3d_triangle3d_clipped_design clipped_design;
            clipped_design.original_color_of = triangle->color_of;
            clipped_design.original_design = triangle->design_data;

            wf3d_triangle3d clipped_triangle;
            clipped_triangle.color_of = wf3d_triangle3d_ClippedDesignCallback;
            clipped_triangle.design_data = &clipped_design;
            clipped_triangle.normal = rel_normal;

            for(int vi = 0 ; vi < 3 ; vi++)
            {
                for(int k = 0 ; k < 3 ; k++)
                {
                    clipped_design.vertices_barycentric_coords[vi][k] = 0.0;
                }
            }

            int vi_unclipped = 0;
            for(int vi = 0 ; vi < 3 ; vi++)
            {
                if(!vertex_is_behind[vi])
                {
                    vi_unclipped = vi;
                }
            }

            for(int vi = 0 ; vi < 3 ; vi++)
            {
                if(vertex_is_behind[vi])
                {
                    float const t = - (vertex_coords[vi_unclipped][3] + cam->near_clipping_distance) / (vertex_coords[vi][3] - vertex_coords[vi_unclipped][3]);
                    wf3d_vect3d clipped_vertex = wf3d_vect3d_add(
                                                                    wf3d_vect3d_scalar_mul(rel_vertex[vi], t),
                                                                    wf3d_vect3d_scalar_mul(rel_vertex[vi_unclipped], 1.0f - t)
                                                                );
                    clipped_triangle.vertex_list[vi] = wf3d_vect3d_set_component(clipped_vertex, 2, -cam->near_clipping_distance);
                    clipped_design.vertices_barycentric_coords[vi][vi] = t;
                    clipped_design.vertices_barycentric_coords[vi][vi_unclipped] = 1.0f - t;
                }
                else
                {
                    clipped_triangle.vertex_list[vi] = rel_vertex[vi];
                    clipped_design.vertices_barycentric_coords[vi][vi] = 1.0f;
                }
            }

            wf3d_vect3d const clipped_v_pos = wf3d_vect3d_zero();
            wf3d_vect3d const clipped_q_rot = wf3d_quat_from_real(1.0f);
            error = wf3d_triangle3d_Rasterization(&clipped_triangle, img_out, clipped_v_pos, clipped_q_rot, cam);
        }
    }

    return error;
}

//design_data_ptr : wf3d_color*
//
//
wf3d_color* wf3d_triangle3d_MonoColorSurfaceCallback(void const* design_data_ptr, wf3d_color* color_ret, float const* barycentric_coords)
{
    if(design_data_ptr == NULL)
    {
        return NULL;
    }

    wf3d_color const* design_data = design_data_ptr;
    *color_ret = *design_data;

    return color_ret;
}

//design_data_ptr : wf3d_triangle3d_tricolor_design*
//
//
wf3d_color* wf3d_triangle3d_TriColorSurfaceCallback(void const* design_data_ptr, wf3d_color* color_ret, float const* barycentric_coords)
{
    if(design_data_ptr == NULL)
    {
        return NULL;
    }

    wf3d_triangle3d_tricolor_design const* design_data = design_data_ptr;

    wf3d_color_mix_colors(color_ret, design_data->vertex_color_list, barycentric_coords, 3);

    return color_ret;
}

//design_data_ptr : wf3d_triangle3d_clipped_design*
//
//
wf3d_color* wf3d_triangle3d_ClippedDesignCallback(void const* design_data_ptr, wf3d_color* color_ret, float const* barycentric_coords)
{
    if(design_data_ptr == NULL)
    {
        return NULL;
    }

    wf3d_triangle3d_clipped_design const* design_data = design_data_ptr;

    float new_barycentric_coords[3];
    for(int i = 0 ; i < 3 ; i++)
    {
        new_barycentric_coords[i] = 0.0;

        for(int j = 0 ; j < 3 ; j++)
        {
            new_barycentric_coords[i] += barycentric_coords[j] * design_data->vertices_barycentric_coords[j][i];
        }
    }

    return design_data->original_color_of(design_data->original_design, color_ret, new_barycentric_coords);
}
