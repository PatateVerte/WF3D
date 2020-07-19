#include <WF3D/Rendering/Shapes/triangle3d.h>

//
//
//
wf3d_triangle3d* wf3d_triangle3d_Set(
                                        wf3d_triangle3d* triangle, owl_v3f32* vertex_list, owl_v3f32 normal,
                                        wf3d_surface* (*surface_of)(void const*, wf3d_surface*, float const*),
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

    triangle->surface_of = surface_of;
    triangle->design_data = design_data;

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

    return wf3d_triangle3d_CopyDesign(t_dst, t_src);
}

//
//
//
wf3d_triangle3d* wf3d_triangle3d_GeometricTransform(wf3d_triangle3d* t_dst, wf3d_triangle3d const* t_src, owl_q32 q_rot, owl_v3f32 v)
{
    for(int i = 0 ; i < 3 ; i++)
    {
        t_dst->vertex_list[i] = owl_q32_add(
                                                owl_q32_transform_v3f32(q_rot, t_src->vertex_list[i]),
                                                v
                                             );
    }
    t_dst->normal = owl_q32_transform_v3f32(q_rot, t_src->normal);

    return t_dst;
}

//
//
//
wf3d_triangle3d* wf3d_triangle3d_CopyAndTransform(wf3d_triangle3d* t_dst, wf3d_triangle3d const* t_src, owl_q32 q_rot, owl_v3f32 v)
{
    for(int i = 0 ; i < 3 ; i++)
    {
        t_dst->vertex_list[i] = owl_q32_add(
                                                owl_q32_transform_v3f32(q_rot, t_src->vertex_list[i]),
                                                v
                                             );
    }
    t_dst->normal = owl_q32_transform_v3f32(q_rot, t_src->normal);

    return wf3d_triangle3d_CopyDesign(t_dst, t_src);
}

//The intersection between a ray and the triangle3d
//Return true if the intersection exists and returns the parameter, false otherwise
//Optional parameters (modified only if an intersection has been found) :
//t to return the parameter for the nearest intersection (v_intersection = ray_origin + t*ray_dir)
//normal_ret to return the normal of the intersection
//surface_ret to return the surface of the intersection
bool wf3d_triangle3d_NearestIntersectionWithRay(wf3d_triangle3d const* triangle, owl_v3f32 v_pos, owl_q32 q_rot, owl_v3f32 ray_origin, owl_v3f32 ray_dir, float t_min, float t_max, float* t_ret, owl_v3f32* normal_ret, wf3d_surface* surface_ret)
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
        float det_sum = 0.0;
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
            det_sum += det[k];
            same_sign = (det[k] * det[k2] >= 0.0);
        }

        if(same_sign)
        {
            float barycentric_coords[3];
            for(unsigned int k = 0 ; k < 3 ; k++)
            {
                barycentric_coords[k] = det[k] / det_sum;
            }

            intersection_found = true;

            if(t_ret != NULL)
            {
                *t_ret = t;
            }

            if(normal_ret != NULL)
            {
                *normal_ret = owl_q32_transform_v3f32(q_rot, triangle->normal);
            }

            if(surface_ret != NULL)
            {
                triangle->surface_of(triangle->design_data, surface_ret, barycentric_coords);
            }
        }
    }

    return intersection_found;
}

//triangle_ret_list[2]
//
//
unsigned int wf3d_triangle3d_Clipping(wf3d_triangle3d* clipped_triangle, wf3d_triangle3d_clipped_design* clipped_design, wf3d_triangle3d const* triangle, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam)
{
    owl_v3f32 rel_vertex[3];
    float vertex_coords[3][4] OWL_ALIGN16;

    //Clipping detection
    unsigned int nb_vertices_behind = 0;
    bool vertex_is_behind[3];

    for(unsigned int vi = 0 ; vi < 3 ; vi++)
    {
        rel_vertex[vi] = owl_v3f32_add(
                                        owl_q32_transform_v3f32(q_rot, triangle->vertex_list[vi]),
                                        v_pos
                                       );
        //Storing
        owl_v3f32_store4(vertex_coords[vi], rel_vertex[vi]);
        clipped_triangle[0].vertex_list[vi] = rel_vertex[vi];
        float const z_vertex = owl_v3f32_unsafe_get_component(rel_vertex[vi], 2);

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

    owl_v3f32 rel_normal = owl_q32_transform_v3f32(q_rot, triangle->normal);

    //If there is no need for clipping
    if(nb_vertices_behind == 0)
    {
        clipped_triangle[0].normal = rel_normal;
        wf3d_triangle3d_CopyDesign(clipped_triangle + 0, triangle);
        return 1;
    }
    //If there is only one vertex behind
    else if(nb_vertices_behind == 1)
    {
        for(int p = 0 ; p < 2 ; p++)
        {
            clipped_design[p].original_surface_of = triangle->surface_of;
            clipped_design[p].original_design = triangle->design_data;

            clipped_triangle[p].surface_of = &wf3d_triangle3d_ClippedDesignCallback;
            clipped_triangle[p].design_data = clipped_design + p;
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

            float const t = (vertex_coords[vi_clipped][2] + cam->near_clipping_distance) / (vertex_coords[vi_clipped][2] - vertex_coords[vi][2]);
            owl_v3f32 clipped_vertex = owl_v3f32_add(
                                                            owl_v3f32_scalar_mul(rel_vertex[vi], t),
                                                            owl_v3f32_scalar_mul(rel_vertex[vi_clipped], 1.0f - t)
                                                         );
            clipped_vertex = owl_v3f32_unsafe_set_component(clipped_vertex, 2, -cam->near_clipping_distance);

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

        return 2;
    }
    //If there are 2 vertices behind
    else if(nb_vertices_behind == 2)
    {
        clipped_design[0].original_surface_of = triangle->surface_of;
        clipped_design[0].original_design = triangle->design_data;

        clipped_triangle[0].surface_of = &wf3d_triangle3d_ClippedDesignCallback;
        clipped_triangle[0].design_data = clipped_design + 0;
        clipped_triangle[0].normal = rel_normal;

        for(int vi = 0 ; vi < 3 ; vi++)
        {
            for(int k = 0 ; k < 3 ; k++)
            {
                clipped_design[0].vertices_barycentric_coords[vi][k] = 0.0;
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
                float const t = - (vertex_coords[vi_unclipped][2] + cam->near_clipping_distance) / (vertex_coords[vi][2] - vertex_coords[vi_unclipped][2]);
                owl_v3f32 clipped_vertex = owl_v3f32_add(
                                                                owl_v3f32_scalar_mul(rel_vertex[vi], t),
                                                                owl_v3f32_scalar_mul(rel_vertex[vi_unclipped], 1.0f - t)
                                                            );
                clipped_triangle[0].vertex_list[vi] = owl_v3f32_unsafe_set_component(clipped_vertex, 2, -cam->near_clipping_distance);
                clipped_design[0].vertices_barycentric_coords[vi][vi] = t;
                clipped_design[0].vertices_barycentric_coords[vi][vi_unclipped] = 1.0f - t;
            }
            else
            {
                clipped_triangle[0].vertex_list[vi] = rel_vertex[vi];
                clipped_design[0].vertices_barycentric_coords[vi][vi] = 1.0f;
            }
        }
        return 1;
    }
    else
    {
        return 0;
    }
}

//
//
//
wf3d_error wf3d_triangle3d_RasterizationAfterClipping(wf3d_triangle3d const* triangle, wf3d_image2d_rectangle* img_out, wf3d_rasterization_env const* env)
{
    wf3d_error error = WF3D_SUCCESS;

    wf3d_camera3d const* cam = env->cam;

    float half_width = 0.5 * (float)img_out->img2d->width;
    float half_height = 0.5 * (float)img_out->img2d->height;

    float const x_scale = half_width / cam->tan_h_half_opening_angle;
    float const y_scale = half_height / cam->tan_v_half_opening_angle;
    float const inv_x_scale = 1.0 / x_scale;
    float const inv_y_scale = 1.0 / y_scale;

    owl_v3f32 rel_vertex[3];
    float vertex_coords[3][4] OWL_ALIGN16;

    owl_v3f32 opp_side_cross_normal[3];
    owl_v3f32 rel_normal = triangle->normal;

    float triangle_min_depth = INFINITY;

    for(int vi = 0 ; vi < 3 ; vi++)
    {
        rel_vertex[vi] = triangle->vertex_list[vi];
        //Storing
        owl_v3f32_store4(vertex_coords[vi], rel_vertex[vi]);
        float const z_vertex = vertex_coords[vi][2];

        //Min depth
        triangle_min_depth = fminf(triangle_min_depth, -z_vertex);
    }

    for(int vi = 0 ; vi < 3 ; vi++)
    {
        int const vi1 = (vi + 1) % 3;
        int const vi2 = (vi + 2) % 3;
        opp_side_cross_normal[vi] = owl_v3f32_cross(
                                                        rel_normal,
                                                        owl_v3f32_sub(rel_vertex[vi2], rel_vertex[vi1])
                                                      );
    }

    bool blackface_result = true;
    if(cam->blackface_culling_enabled)
    {
        for(int vi = 0 ; vi < 3 && blackface_result ; vi++)
        {
            blackface_result = owl_v3f32_dot(rel_vertex[vi], rel_normal) < 0.0;
        }
    }

    if(blackface_result)
    {
        float screen_coords[3][2];

        float x_gen_min_f = 2.0 * half_width;
        float x_gen_max_f = 0.0;
        float y_min_f = 2.0 * half_height;
        float y_max_f = 0.0;

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

        if(x_gen_min_f < 2.0 * half_width && x_gen_max_f >= 0.0)
        {
            int y_min = (int)roundf(fmaxf(y_min_f, 0.0));
            if(y_min < img_out->y_min)
            {
                y_min = img_out->y_min;
            }
            int y_max = (int)roundf(fminf(y_max_f, 2.0f * half_height));
            if(y_max > img_out->y_max)
            {
                y_max = img_out->y_max;
            }


            for(int y = y_min ; y < y_max && error == WF3D_SUCCESS; y++)
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
                if(x_min < img_out->x_min)
                {
                    x_min = img_out->x_min;
                }
                int x_max = (int)roundf(fminf(x_max_f, 2.0f * half_width));
                if(x_max > img_out->x_max)
                {
                    x_max = img_out->x_max;
                }

                for(int x = x_min ; x < x_max && error == WF3D_SUCCESS ; x++)
                {
                    if(triangle_min_depth <= wf3d_Image2d_unsafe_Depth(img_out->img2d, x, y))
                    {
                        float x_f = 0.5f + (float)x;

                        owl_v3f32 v_intersection;
                        {
                            owl_v3f32 dir_vect = owl_v3f32_set(
                                                                (x_f - half_width) * inv_x_scale,
                                                                (y_f - half_height) * inv_y_scale,
                                                                -1.0
                                                               );

                            float const t = owl_v3f32_dot(rel_vertex[0], rel_normal) / owl_v3f32_dot(dir_vect, rel_normal);
                            v_intersection = owl_v3f32_scalar_mul(dir_vect, t);
                        }

                        float depth = - owl_v3f32_unsafe_get_component(v_intersection, 2);

                        if(depth <= cam->far_clipping_distance && depth <= wf3d_Image2d_unsafe_Depth(img_out->img2d, x, y))
                        {
                            //Gets barycentric coordinates
                            float barycentric_coords[3];
                            float barycentric_sum = 0.0;
                            for(int vi = 0 ; vi < 3 ; vi++)
                            {
                                int const vi1 = (vi + 1) % 3;
                                float const tmp = owl_v3f32_dot(
                                                                    opp_side_cross_normal[vi],
                                                                    owl_v3f32_sub(v_intersection, rel_vertex[vi1])
                                                                 );
                                barycentric_coords[vi] = tmp;
                                barycentric_sum += tmp;
                            }

                            float const inv_barycentric_sum = 1.0f / barycentric_sum;
                            for(int vi = 0 ; vi < 3 ; vi++)
                            {
                                barycentric_coords[vi] *= inv_barycentric_sum;
                            }

                            wf3d_surface surface;
                            triangle->surface_of(triangle->design_data, &surface, barycentric_coords);

                            wf3d_color final_color;
                            wf3d_lightsource_enlight_surface(env->lightsource_list, env->nb_lightsources, &final_color, &surface, v_intersection, rel_normal);

                            error = wf3d_Image2d_SetPixel(img_out->img2d, x, y, &final_color, depth);
                        }
                    }
                }
            }
        }
    }

    return error;
}

//Rasterization of a triangle
//
//
wf3d_error wf3d_triangle3d_Rasterization(wf3d_triangle3d const* triangle, wf3d_image2d_rectangle* img_out, wf3d_rasterization_env const* env, owl_v3f32 v_pos, owl_q32 q_rot)
{
    wf3d_error error = WF3D_SUCCESS;

    wf3d_triangle3d clipped_triangle[2];
    wf3d_triangle3d_clipped_design clipped_design[2];

    unsigned int nb_clipped_triangle = wf3d_triangle3d_Clipping(clipped_triangle, clipped_design, triangle, v_pos, q_rot, env->cam);

    for(unsigned int k = 0 ; k < nb_clipped_triangle && error == WF3D_SUCCESS ; k++)
    {
        wf3d_triangle3d_RasterizationAfterClipping(clipped_triangle + k, img_out, env);
    }

    return error;
}

//
//
//
wf3d_error wf3d_triangle3d_Rasterization2AfterClipping(wf3d_triangle3d const* triangle, wf3d_image3d_image_piece* img_out, wf3d_camera3d const* cam)
{
    wf3d_error error = WF3D_SUCCESS;

    float half_width = 0.5 * (float)img_out->full_img_width;
    float half_height = 0.5 * (float)img_out->full_img_height;

    float const x_scale = half_width / cam->tan_h_half_opening_angle;
    float const y_scale = half_height / cam->tan_v_half_opening_angle;
    float const inv_x_scale = 1.0 / x_scale;
    float const inv_y_scale = 1.0 / y_scale;

    owl_v3f32 rel_vertex[3];
    float vertex_coords[3][4] OWL_ALIGN16;

    owl_v3f32 opp_side_cross_normal[3];
    owl_v3f32 rel_normal = triangle->normal;

    float triangle_min_depth = INFINITY;

    for(int vi = 0 ; vi < 3 ; vi++)
    {
        rel_vertex[vi] = triangle->vertex_list[vi];
        //Storing
        owl_v3f32_store4(vertex_coords[vi], rel_vertex[vi]);
        float const z_vertex = vertex_coords[vi][2];

        //Min depth
        triangle_min_depth = fminf(triangle_min_depth, -z_vertex);
    }

    for(int vi = 0 ; vi < 3 ; vi++)
    {
        int const vi1 = (vi + 1) % 3;
        int const vi2 = (vi + 2) % 3;
        opp_side_cross_normal[vi] = owl_v3f32_cross(
                                                        rel_normal,
                                                        owl_v3f32_sub(rel_vertex[vi2], rel_vertex[vi1])
                                                      );
    }

    bool blackface_result = true;
    if(cam->blackface_culling_enabled)
    {
        for(int vi = 0 ; vi < 3 && blackface_result ; vi++)
        {
            blackface_result = owl_v3f32_dot(rel_vertex[vi], rel_normal) < 0.0;
        }
    }

    if(blackface_result)
    {
        float screen_coords[3][2];

        float x_gen_min_f = 2.0 * half_width;
        float x_gen_max_f = 0.0;
        float y_min_f = 2.0 * half_height;
        float y_max_f = 0.0;

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

        if(x_gen_min_f < 2.0 * half_width && x_gen_max_f >= 0.0)
        {
            int y_min = (int)roundf(fmaxf(y_min_f, 0.0));
            if(y_min < img_out->y_min)
            {
                y_min = img_out->y_min;
            }
            int y_max = (int)roundf(fminf(y_max_f, 2.0f * half_height));
            if(y_max > img_out->y_max)
            {
                y_max = img_out->y_max;
            }


            for(int y = y_min ; y < y_max && error == WF3D_SUCCESS; y++)
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
                if(x_min < img_out->x_min)
                {
                    x_min = img_out->x_min;
                }
                int x_max = (int)roundf(fminf(x_max_f, 2.0f * half_width));
                if(x_max > img_out->x_max)
                {
                    x_max = img_out->x_max;
                }

                for(int x = x_min ; x < x_max && error == WF3D_SUCCESS ; x++)
                {
                    if(triangle_min_depth <= wf3d_Image3d_unsafe_Depth(img_out->img3d, x - img_out->x_min, y - img_out->y_min))
                    {
                        float x_f = 0.5f + (float)x;

                        owl_v3f32 v_intersection;
                        {
                            owl_v3f32 dir_vect = owl_v3f32_set(
                                                                (x_f - half_width) * inv_x_scale,
                                                                (y_f - half_height) * inv_y_scale,
                                                                -1.0
                                                               );

                            float const t = owl_v3f32_dot(rel_vertex[0], rel_normal) / owl_v3f32_dot(dir_vect, rel_normal);
                            v_intersection = owl_v3f32_scalar_mul(dir_vect, t);
                        }

                        float depth = - owl_v3f32_unsafe_get_component(v_intersection, 2);

                        if(depth <= cam->far_clipping_distance && depth <= wf3d_Image3d_unsafe_Depth(img_out->img3d, x - img_out->x_min, y - img_out->y_min))
                        {
                            //Gets barycentric coordinates
                            float barycentric_coords[3];
                            float barycentric_sum = 0.0;
                            for(int vi = 0 ; vi < 3 ; vi++)
                            {
                                int const vi1 = (vi + 1) % 3;
                                float const tmp = owl_v3f32_dot(
                                                                    opp_side_cross_normal[vi],
                                                                    owl_v3f32_sub(v_intersection, rel_vertex[vi1])
                                                                 );
                                barycentric_coords[vi] = tmp;
                                barycentric_sum += tmp;
                            }

                            float const inv_barycentric_sum = 1.0f / barycentric_sum;
                            for(int vi = 0 ; vi < 3 ; vi++)
                            {
                                barycentric_coords[vi] *= inv_barycentric_sum;
                            }

                            wf3d_surface surface;
                            triangle->surface_of(triangle->design_data, &surface, barycentric_coords);

                            error = wf3d_Image3d_SetPixel(img_out->img3d, x - img_out->x_min, y - img_out->y_min, &surface, depth, v_intersection, rel_normal);
                        }
                    }
                }
            }
        }
    }

    return error;
}

//
//
//
wf3d_error wf3d_triangle3d_Rasterization2(wf3d_triangle3d const* triangle, wf3d_image3d_image_piece* img_out, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam)
{
    wf3d_error error = WF3D_SUCCESS;

    wf3d_triangle3d clipped_triangle[2];
    wf3d_triangle3d_clipped_design clipped_design[2];

    unsigned int nb_clipped_triangle = wf3d_triangle3d_Clipping(clipped_triangle, clipped_design, triangle, v_pos, q_rot, cam);

    for(unsigned int k = 0 ; k < nb_clipped_triangle && error == WF3D_SUCCESS ; k++)
    {
        wf3d_triangle3d_Rasterization2AfterClipping(clipped_triangle + k, img_out, cam);
    }

    return error;
}

//design_data_ptr : wf3d_surface*
//
//
wf3d_surface* wf3d_triangle3d_MonoColorSurfaceCallback(void const* design_data_ptr, wf3d_surface* surface_ret, float const* barycentric_coords)
{
    (void)barycentric_coords;
    wf3d_surface const* design_data = design_data_ptr;
    *surface_ret = *design_data;

    return surface_ret;
}

//design_data_ptr : wf3d_triangle3d_tricolor_design*
//
//
wf3d_surface* wf3d_triangle3d_TriColorSurfaceCallback(void const* design_data_ptr, wf3d_surface* surface_ret, float const* barycentric_coords)
{
    wf3d_triangle3d_tricolor_design const* design_data = design_data_ptr;
    return wf3d_surface_mix(surface_ret, design_data->vertex_surface_list, barycentric_coords, 3);
}

//design_data_ptr : wf3d_triangle3d_clipped_design*
//
//
wf3d_surface* wf3d_triangle3d_ClippedDesignCallback(void const* design_data_ptr, wf3d_surface* surface_ret, float const* barycentric_coords)
{
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

    return design_data->original_surface_of(design_data->original_design, surface_ret, new_barycentric_coords);
}
