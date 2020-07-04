#include <WF3D/Rendering/camera3d.h>

#include <math.h>

//
//
//
wf3d_camera3d* wf3d_camera3d_Set(wf3d_camera3d* cam, float near_clipping_distance, float far_clipping_distance, float h_opening_angle, float v_opening_angle, bool blackface_culling_enabled)
{
    cam->near_clipping_distance = near_clipping_distance;
    cam->far_clipping_distance = far_clipping_distance;
    cam->tan_h_half_opening_angle = tanf(0.5f * h_opening_angle);
    cam->tan_v_half_opening_angle = tanf(0.5f * v_opening_angle);
    cam->blackface_culling_enabled = blackface_culling_enabled;

    return cam;
}
