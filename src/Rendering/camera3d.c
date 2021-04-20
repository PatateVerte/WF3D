#include <WF3D/Rendering/camera3d.h>

#include <math.h>

//
//
//
OWL_DLL_EXPORT wf3d_camera3d* wf3d_camera3d_Set(wf3d_camera3d* cam, float near_clipping_distance, float h_opening_angle, float v_opening_angle, bool blackface_culling_enabled, int max_nb_reflections)
{
    cam->near_clipping_distance = near_clipping_distance;
    cam->tan_h_half_opening_angle = tanf(0.5f * h_opening_angle);
    cam->tan_v_half_opening_angle = tanf(0.5f * v_opening_angle);
    cam->blackface_culling_enabled = blackface_culling_enabled;
    cam->max_nb_reflections = max_nb_reflections;

    return cam;
}
