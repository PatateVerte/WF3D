#include <WF3D/Rendering/camera3d.h>

//
//
//
wf3d_camera3d* wf3d_camera3d_Set(wf3d_camera3d* cam, float render_distance, float h_opening_angle, float v_opening_angle, bool blackface_culling_enabled)
{
    cam->render_distance = render_distance;
    cam->h_opening_angle = h_opening_angle;
    cam->v_opening_angle = v_opening_angle;
    cam->blackface_culling_enabled = blackface_culling_enabled;

    cam->q_rot = wf3d_quat_from_real(1.0f);
    cam->v_pos = wf3d_vect3d_zero();

    return cam;
}

//
//
//
wf3d_error wf3d_camera3d_Rasterization(wf3d_camera3d* cam, wf3d_img_gen_interface* img_out, float* z_buffer, wf3d_Object3d const** scene, size_t nb_objects)
{
    wf3d_error error = WF3D_SUCCESS;

    float tan_h_half_opening_angle = tanf(0.5f * cam->h_opening_angle);
    float tan_v_half_opening_angle = tanf(0.5f * cam->v_opening_angle);

    for(size_t k = 0 ; k < nb_objects && error == WF3D_SUCCESS ; k++)
    {
        //Rasterization of the object
        error = wf3d_Object3d_Rasterization(scene[k], img_out, z_buffer, cam->v_pos, cam->q_rot,
                                            tan_h_half_opening_angle, tan_v_half_opening_angle,
                                            cam->render_distance, cam->blackface_culling_enabled);
    }

    return error;
}
