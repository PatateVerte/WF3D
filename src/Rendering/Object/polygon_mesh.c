#include <WF3D/Rendering/Object/polygon_mesh.h>

#include <math.h>
#include <malloc.h>
#include <WF3D/const.h>

//
//
//
wf3d_PolygonMesh* wf3d_PolygonMesh_Create(int nb_faces)
{
    wf3d_PolygonMesh* obj = NULL;
    bool error = false;

    obj = malloc(sizeof(*obj));
    if(obj != NULL)
    {
        obj->nb_faces = nb_faces;
        obj->local_face_list = NULL;

		obj->radius = 0.0;
		obj->radius_has_changed = true;

        if(nb_faces >= 0)
        {
            size_t face_buff_size = (size_t)nb_faces * sizeof(wf3d_triangle3d);
            obj->local_face_list = _aligned_malloc(face_buff_size, 16);

            if(obj->local_face_list == NULL)
            {
                error = true;
            }
        }
        else
        {
            error = true;
        }
    }
    else
    {
        error = true;
    }

    if(error)
    {
        wf3d_PolygonMesh_Destroy(obj);
        obj = NULL;
    }

    return obj;
}

//
//
//
void wf3d_PolygonMesh_Destroy(wf3d_PolygonMesh* obj)
{
    if(obj != NULL)
    {
        _aligned_free(obj->local_face_list);
        free(obj);
    }
}

//
//
//
wf3d_triangle3d const* wf3d_PolygonMesh_ChangeFace(wf3d_PolygonMesh* obj, int i, wf3d_triangle3d const* new_face)
{
    if(i >= 0 && i < obj->nb_faces)
    {
        wf3d_triangle3d* local_face = obj->local_face_list + i;
        *local_face = *new_face;

        obj->radius_has_changed = true;

        return local_face;
    }
    else
    {
        return NULL;
    }
}

//
//
//
float wf3d_PolygonMesh_Radius(wf3d_PolygonMesh* obj)
{
    if(obj->radius_has_changed)
    {
        float square_radius = 0.0;

        for(int fi = 0 ; fi < obj->nb_faces ; fi++)
        {
            for(int k = 0 ; k < 3 ; k++)
            {
                wf3d_vect3d vertex = obj->local_face_list[fi].vertex_list[k];
                float tmp = wf3d_vect3d_dot(vertex, vertex);
                square_radius = fmaxf(square_radius, tmp);
            }
        }

        float radius = sqrtf(square_radius);
        obj->radius = radius;
        obj->radius_has_changed = false;
        return radius;
    }
    else
    {
        return obj->radius;
    }
}

//
//
//
float wf3d_PolygonMesh_InfRadius(wf3d_PolygonMesh* obj, wf3d_vect3d v_pos)
{
    float inf_radius = 0.0;

    for(int fi = 0 ; fi < obj->nb_faces ; fi++)
    {
        for(int k = 0 ; k < 3 ; k++)
        {
            wf3d_vect3d vertex = obj->local_face_list[fi].vertex_list[k];
            float tmp = wf3d_vect3d_inf_norm( wf3d_vect3d_add(vertex, v_pos) );
            inf_radius = fmaxf(inf_radius, tmp);
        }
    }

    return inf_radius;
}

//
//
//
float wf3d_PolygonMesh_InfRadiusWithRot(wf3d_PolygonMesh* obj, wf3d_vect3d v_pos, wf3d_quat q_rot)
{
    float inf_radius = 0.0;

    for(int fi = 0 ; fi < obj->nb_faces ; fi++)
    {
        for(int k = 0 ; k < 3 ; k++)
        {
            wf3d_vect3d vertex = obj->local_face_list[fi].vertex_list[k];
            float tmp = wf3d_vect3d_inf_norm( wf3d_vect3d_add( wf3d_quat_transform_vect3d(q_rot, vertex), v_pos) );
            inf_radius = fmaxf(inf_radius, tmp);
        }
    }

    return inf_radius;
}

//
//
//
wf3d_error wf3d_PolygonMesh_Rasterization(wf3d_PolygonMesh const* obj, wf3d_img_gen_interface* img_out, float* depth_buffer, wf3d_vect3d v_pos, wf3d_quat q_rot, wf3d_camera3d const* cam)
{
    wf3d_error error = WF3D_SUCCESS;

    wf3d_quat const cam_rot_conj = wf3d_quat_conj(cam->q_rot);

    wf3d_quat q_inv = wf3d_quat_mul(
                                        cam_rot_conj,
                                        q_rot
                                      );
    wf3d_quat v_inv = wf3d_quat_transform_vect3d(
                                                        cam_rot_conj,
                                                        wf3d_vect3d_sub(v_pos, cam->v_pos)
                                                    );

    float half_width = 0.5f * (float)img_out->width;
    float half_height = 0.5f * (float)img_out->height;

    float x_scale = half_width / cam->tan_h_half_opening_angle;
    float y_scale = half_height / cam->tan_v_half_opening_angle;

    for(int fi = 0 ; fi < obj->nb_faces && error == WF3D_SUCCESS ; fi++)
    {
        wf3d_triangle3d const* local_face = obj->local_face_list + fi;

        float screen_coords[3][4] __attribute__( (aligned(16)) );
        float y_min_f = 2.0f * half_height;
        float y_max_f = 0.0;

        for(int i = 0 ; i < 3 ; i++)
        {
            wf3d_vect3d_store4(
                                screen_coords[i],
                                wf3d_vect3d_add(
                                                wf3d_quat_transform_vect3d(q_inv, local_face->vertex_list[i]),
                                                v_inv
                                               )
                              );

            float tmp = -1.0f / screen_coords[i][3];
            screen_coords[i][1] = screen_coords[i][1] * tmp * x_scale + half_width;
            screen_coords[i][2] = screen_coords[i][2] * tmp * y_scale + half_height;

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
                    local_face->color_of(local_face->design_data, &final_color, barycentric_coords);

                    error = img_out->set_pixel_callback_z_buffer(img_out->img_obj, x, y, &final_color, depth);
                }
            }
        }

    }

    return error;
}
