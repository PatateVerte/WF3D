#include <WF3D/Rendering/Shapes/triangle3d.h>

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
wf3d_error wf3d_triangle3d_Rasterization(wf3d_triangle3d const* triangle, wf3d_img_gen_interface* img_out, float* depth_buffer, wf3d_vect3d v_pos, wf3d_quat q_rot, wf3d_camera3d const* cam)
{
    if(triangle == NULL)
    {
        return WF3D_SUCCESS;
    }

    wf3d_error error = WF3D_SUCCESS;

    float half_width = 0.5f * (float)img_out->width;
    float half_height = 0.5f * (float)img_out->height;

    float x_scale = half_width / cam->tan_h_half_opening_angle;
    float y_scale = half_height / cam->tan_v_half_opening_angle;

    float vertex_coords[3][4] __attribute__( (aligned(16)) );
    float y_min_f = 2.0f * half_height;
    float y_max_f = 0.0;

    //Clipping detection
    int nb_vertices_behind = 0;
    bool is_behind[3];

    for(int i = 0 ; i < 3 ; i++)
    {
        //Storing
        wf3d_vect3d_store4(
                            vertex_coords[i],
                            wf3d_vect3d_add(
                                            wf3d_quat_transform_vect3d(q_rot, triangle->vertex_list[i]),
                                            v_pos
                                           )
                          );

        //Clipping
        if(vertex_coords[i][3] < 0.0)
        {
            is_behind[i] = true;
            nb_vertices_behind += 1;
        }
        else
        {
            is_behind[i] = false;
        }
    }

    //If there is no need for clipping
    if(nb_vertices_behind == 0)
    {
        float screen_coords[3][4] __attribute__( (aligned(16)) );

        for(int i = 0 ; i < 3 ; i++)
        {
            float tmp = -1.0f / vertex_coords[i][3];
            screen_coords[i][1] = vertex_coords[i][1] * tmp * x_scale + half_width;
            screen_coords[i][2] = vertex_coords[i][2] * tmp * y_scale + half_height;

            y_min_f = fminf(y_min_f, screen_coords[i][2]);
            y_max_f = fmaxf(y_max_f, screen_coords[i][2]);
        }

        int y_min = (int)roundf(fmaxf(y_min_f, 0.0));
        if(y_min < 0)
        {
            y_min = 0;
        }
        int y_max = (int)roundf(fminf(y_max_f, 2.0f * half_height));
        if(y_max > img_out->height)
        {
            y_max = img_out->height;
        }

        for(int y = y_min ; y < y_max && error == WF3D_SUCCESS; y++)
        {
            float y_f = 0.5f + (float)y;

            float x_min_f = 2.0f * half_width;
            float x_max_f = 0.0;
            float x_min_coeff[3] = {0.0};
            float x_max_coeff[3] = {0.0};
            float x_min_z = -1.0f;
            float x_max_z = -1.0f;
            for(int i0 = 0 ; i0 < 3 ; i0++)
            {
                int i1 = (i0 + 1) % 3;
                int i2 = (i0 + 2) % 3;
                float t = (y_f - screen_coords[i1][2]) / (screen_coords[i0][2] - screen_coords[i1][2]);
                if(0.0 <= t && t <= 1.0f)
                {
                    float a = t;
                    float b = 1.0f - t;
                    float x_f = a * screen_coords[i0][1] + b * screen_coords[i1][1];

                    if(x_f < x_min_f)
                    {
                        x_min_f = x_f;
                        x_min_coeff[i0] = a;
                        x_min_coeff[i1] = b;
                        x_min_coeff[i2] = 0.0;
                        x_min_z = -(a * screen_coords[i0][3] + b * screen_coords[i1][3]);
                    }

                    if(x_f > x_max_f)
                    {
                        x_max_f = x_f;
                        x_max_coeff[i0] = a;
                        x_max_coeff[i1] = b;
                        x_max_coeff[i2] = 0.0;
                        x_max_z = -(a * screen_coords[i0][3] + b * screen_coords[i1][3]);
                    }
                }
            }

            int x_min = (int)roundf(fmaxf(x_min_f, 0.0));
            if(x_min < 0)
            {
                x_min = 0;
            }
            int x_max = (int)roundf(fminf(x_max_f, 2.0f * half_width));
            if(x_max > img_out->width)
            {
                x_max = img_out->width;
            }

            float inv_delta_x = 1.0f / (x_max_f - x_min_f);

            for(int x = x_min ; x < x_max && error == WF3D_SUCCESS ; x++)
            {
                float x_f = 0.5f + (float)x;
                float t = (x_max_f - x_f) * inv_delta_x;
                float a = t;
                float b = 1.0f - t;

                float depth = a * x_min_z + b * x_max_z;
                size_t depth_buffer_index = (size_t)img_out->width * (size_t)y + (size_t)x;

                if(depth >= 0.0 && (depth_buffer == NULL || depth <= depth_buffer[depth_buffer_index]))
                {
                    if(depth_buffer != NULL)
                    {
                        depth_buffer[depth_buffer_index] = depth;
                    }

                    float barycentric_coords[3];
                    for(int i = 0 ; i < 3 ; i++)
                    {
                        barycentric_coords[i] = a * x_min_coeff[i] + b * x_max_coeff[i];
                    }

                    wf3d_color final_color;
                    triangle->color_of(triangle->design_data, &final_color, barycentric_coords);

                    error = img_out->set_pixel_callback_z_buffer(img_out->img_obj, x, y, &final_color, depth);
                }
            }
        }
    }
    //If there is only one vertex behind
    else if(nb_vertices_behind == 1)
    {
        wf3d_triangle3d_clipped_design clipped_design;
        clipped_design.original_color_of = triangle->color_of;
        clipped_design.original_design = triangle->design_data;

        wf3d_triangle3d clipped_triangle;
        clipped_triangle.color_of = wf3d_triangle3d_ClippedDesignCallback;
        clipped_triangle.design_data = &clipped_design;
        clipped_triangle.normal = triangle->normal;

        for(int vi = 0 ; vi < 3 ; vi++)
        {
            for(int k = 0 ; k < 3 ; k++)
            {
                clipped_design.vertices_barycentric_coords[vi][k] = 0.0;
            }

            if(is_behind[vi])
            {
                for(int k = 0 ; k < 2 ; k++)
                {
                    int vk = (vi + 1 + k) % 3;

                    float new_vertex_coords[4] __attribute__( (aligned(16)) );
                    //wf3d_vect3d_store4(new_vertex_coords, triangle->vertex_list[vk]);

                    float const t = (vertex_coords[vk][3]) / (vertex_coords[vk][3] - vertex_coords[vi][3]);
                    clipped_triangle.vertex_list[vi] = wf3d_vect3d_add(
                                                                        wf3d_vect3d_scalar_mul(triangle->vertex_list[vi], t),
                                                                        wf3d_vect3d_scalar_mul(triangle->vertex_list[vk], 1.0f - t)
                                                                      );
                    clipped_design.vertices_barycentric_coords[vi][vi] = t;
                    clipped_design.vertices_barycentric_coords[vi][vk] = 1.0f - t;
                }
            }
            else
            {
                clipped_triangle.vertex_list[vi] = triangle->vertex_list[vi];
                clipped_design.vertices_barycentric_coords[vi][vi] = 1.0;
            }
        }

        wf3d_vect3d const clipped_v_pos = wf3d_vect3d_zero();
        wf3d_vect3d const clipped_q_rot = wf3d_quat_from_real(1.0f);
        error = wf3d_triangle3d_Rasterization(&clipped_triangle, img_out, depth_buffer, clipped_v_pos, clipped_q_rot, cam);
    }

    return error;
}

//design_data_ptr : wf3d_color*
//
//
wf3d_color* wf3d_triangle3d_MonoColorSurfaceCallback(void* design_data_ptr, wf3d_color* color_ret, float const* barycentric_coords)
{
    if(design_data_ptr == NULL)
    {
        return NULL;
    }

    wf3d_color* design_data = design_data_ptr;
    *color_ret = *design_data;

    return color_ret;
}

//design_data_ptr : wf3d_triangle3d_tricolor_design*
//
//
wf3d_color* wf3d_triangle3d_TriColorSurfaceCallback(void* design_data_ptr, wf3d_color* color_ret, float const* barycentric_coords)
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
wf3d_color* wf3d_triangle3d_ClippedDesignCallback(void* design_data_ptr, wf3d_color* color_ret, float const* barycentric_coords)
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
