#include <WF3D/Rendering/Object/debug_line.h>

#include <math.h>

//Create a DebugLine
//
//
wf3d_DebugLine* wf3d_DebugLine_Create(wf3d_vect3d dir_vect, float t_max, float t_min, wf3d_color* color)
{
    wf3d_DebugLine* line = malloc(sizeof(*line));

    if(line  != NULL)
    {
        line->dir_vect = wf3d_vect3d_normalize(dir_vect);
        line->t_max = t_max;
        line->t_min = t_min;
        line->color = color;
    }

    return line;
}

//Destroy a debug line
void wf3d_DebugLine_Destroy(wf3d_DebugLine* line)
{
    if(line != NULL)
    {
        free(line);
    }
}

//
float wf3d_DebugLine_Radius(wf3d_DebugLine* line)
{
    return fmaxf(line->t_max, line->t_min);
}

//
float wf3d_DebugLine_InfRadius(wf3d_DebugLine* line, wf3d_vect3d v_pos)
{
    wf3d_vect3d const dir_vect = line->dir_vect;
    return fmaxf(
                    wf3d_vect3d_inf_norm(wf3d_vect3d_add_scalar_mul(v_pos, dir_vect, line->t_max)),
                    wf3d_vect3d_inf_norm(wf3d_vect3d_add_scalar_mul(v_pos, dir_vect, line->t_min))
                 );
}

//
float wf3d_DebugLine_InfRadiusWithRot(wf3d_DebugLine* line, wf3d_vect3d v_pos, wf3d_quat q_rot)
{
    wf3d_vect3d const dir_vect = wf3d_quat_transform_vect3d(q_rot, line->dir_vect);
    return fmaxf(
                    wf3d_vect3d_inf_norm(wf3d_vect3d_add_scalar_mul(v_pos,  dir_vect, line->t_max)),
                    wf3d_vect3d_inf_norm(wf3d_vect3d_add_scalar_mul(v_pos,  dir_vect, line->t_min))
                 );
}

//Rasterization function
//
wf3d_error wf3d_DebugLine_Rasterization(wf3d_DebugLine const* line, wf3d_img_gen_interface* img_out, float* depth_buffer, wf3d_vect3d v_pos, wf3d_quat q_rot, wf3d_camera3d const* cam)
{
    return WF3D_SUCCESS;

    /*if(line == NULL)
    {
        return WF3D_SUCCESS;
    }

    wf3d_error error = WF3D_SUCCESS;

    float half_width = 0.5f * (float)img_out->width;
    float half_height = 0.5f * (float)img_out->height;

    float const x_scale = half_width / cam->tan_h_half_opening_angle;
    float const y_scale = half_height / cam->tan_v_half_opening_angle;
    float const inv_x_scale = 1.0f / x_scale;
    float const inv_y_scale = 1.0f / y_scale;

    //Line set up
    //
    wf3d_vect3d const rel_dir_vect = wf3d_quat_transform_vect3d(q_rot, line->dir_vect);

    float v_center_coords[4] __attribute__( (aligned(16)) );
    wf3d_vect3d_store4(v_center_coords, v_pos);
    float v_dir_vect_coords[4] __attribute__( (aligned(16)) );
    wf3d_vect3d_store4(v_dir_vect_coords, rel_dir_vect);

    float clip_t_min = line->t_min;
    float clip_t_max = line->t_max;
    {
        float clip_t_lim = - v_center_coords[3] / v_dir_vect_coords[3];
        if(isfinite(clip_t_lim) != 0)
        {
            clip_t_min = fmaxf(clip_t_min, clip_t_lim);
            clip_t_max = fminf(clip_t_max, clip_t_lim);
        }
    }


    //
    float v_extr_screen_coords[2][4] __attribute__( (aligned(16)) );
    wf3d_vect3d_store4( v_extr_screen_coords[0], wf3d_vect3d_add_scalar_mul(v_pos, rel_dir_vect, clip_t_min) );
    wf3d_vect3d_store4( v_extr_screen_coords[1], wf3d_vect3d_add_scalar_mul(v_pos, rel_dir_vect, clip_t_max) );
    for(int ei = 0 ; ei < 2 ; ei++)
    {
        float const tmp = - 1.0f / v_extr_screen_coords[ei][3];
        v_extr_screen_coords[ei][1] = v_extr_screen_coords[ei][1] * tmp * x_scale + half_width;
        v_extr_screen_coords[ei][2] = v_extr_screen_coords[ei][2] * tmp * y_scale + half_height;
    }

    float v_center_screen_coords[2];
    float v_dir_vect_screen_coords[2];

    float t_min = 0.0;
    float t_max = 1.0f;
    //X lim
    {
        float tmp_lim1 = (- v_center_screen_coords[0]) / v_dir_vect_screen_coords[0];
        float tmp_lim2 = (2.0f * half_width - v_center_screen_coords[0]) / v_dir_vect_screen_coords[0];

        t_min = fmaxf(t_min, fminf(tmp_lim1, tmp_lim2));
        t_max = fminf(t_max, fmaxf(tmp_lim1, tmp_lim2));
    }
    //Y lim
    {
        float tmp_lim1 = (- v_center_screen_coords[1]) / v_dir_vect_screen_coords[1];
        float tmp_lim2 = (2.0f * half_height - v_center_screen_coords[1]) / v_dir_vect_screen_coords[1];

        t_min = fmaxf(t_min, fminf(tmp_lim1, tmp_lim2));
        t_max = fminf(t_max, fmaxf(tmp_lim1, tmp_lim2));
    }
    //Lim
    t_min = fminf(t_min, 1.0f);
    t_max = fmaxf(t_max, 0.0f);

    float inv_cos = 1.0 / v_dir_vect_screen_coords[0];
    float inv_sin = 1.0 / v_dir_vect_screen_coords[1];

    float t_step = fminf(fabsf(inv_cos), fabsf(inv_sin));

    if(t_step > 0.0 && t_min < t_max)
    {
        int kt_max = (int)roundf((t_max - t_min) / t_step);
        for(int kt = 0 ; kt <= kt_max && error == WF3D_SUCCESS ; kt++)
        {
            float t = t_min + kt * t_step;

            float x_f = v_center_screen_coords[0] + t * v_dir_vect_screen_coords[0];
            float y_f = v_center_screen_coords[1] + t * v_dir_vect_screen_coords[1];

            int x = (int)roundf(x_f);
            int y = (int)roundf(y_f);

            if(x >= 0 && x < img_out->width && y >= 0 && y < img_out->height)
            {
                float depth = 0.0;
                if(depth >= 0.0)
                {
                    error = img_out->set_pixel_callback_z_buffer(img_out->img_obj, x, y, line->color, depth);
                }
            }
        }
    }

    return error;*/
}

