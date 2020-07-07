#include <WF3D/Rendering/Object/cylinder.h>

//Create an cylinder
//
//
wf3d_Cylinder* wf3d_Cylinder_Create(float r, float h, wf3d_surface const* surface_side, wf3d_surface const* surface_extrem)
{
    wf3d_Cylinder* cylinder = malloc(sizeof(*cylinder));

    if(cylinder != NULL)
    {
        float half_h = 0.5 * h;

        cylinder->r = r;
        cylinder->half_h = half_h;

        float const delta = 1.0 - (1.0 / ((float)(1<<7)));
        owl_q32 q_eigenbasis = owl_q32_from_real(1.0);

        owl_v3f32 norminf_filter_side = owl_v3f32_set(1.0 / half_h, delta / r, delta / r);
        owl_v3f32 norm2_filter_side = owl_v3f32_broadcast(delta / sqrtf(r*r + half_h*half_h));
        float c_side = - r*r;
        owl_v3f32 a_side = owl_v3f32_zero();
        owl_v3f32 alpha_side = owl_v3f32_set(0.0, 1.0, 1.0);
        wf3d_quadratic_curve_set(&cylinder->side, q_eigenbasis, norminf_filter_side, norm2_filter_side, c_side, a_side, alpha_side, surface_side);

        owl_v3f32 norminf_filter_extrem = owl_v3f32_set(delta / half_h, delta / r, delta / r);
        owl_v3f32 norm2_filter_extrem = owl_v3f32_set(0.0, 1.0 / r, 1.0 / r);
        float c_extrem = - half_h * half_h;
        owl_v3f32 a_extrem = owl_v3f32_zero();
        owl_v3f32 alpha_extrem = owl_v3f32_set(1.0, 0.0, 0.0);
        wf3d_quadratic_curve_set(&cylinder->extrem, q_eigenbasis, norminf_filter_extrem, norm2_filter_extrem, c_extrem, a_extrem, alpha_extrem, surface_extrem);
    }

    return cylinder;
}

//
//
//
void wf3d_Cylinder_Destroy(wf3d_Cylinder* cylinder)
{
    if(cylinder != NULL)
    {
        free(cylinder);
    }
}

//
//
//
float wf3d_Cylinder_Radius(wf3d_Cylinder* cylinder)
{
    return sqrtf(cylinder->r * cylinder->r + cylinder->half_h / cylinder->half_h);
}

//
//
//
float wf3d_Cylinder_InfRadius(wf3d_Cylinder* cylinder, owl_v3f32 v_pos)
{
    return fmaxf(cylinder->r, cylinder->half_h);
}

//
float wf3d_Cylinder_InfRadiusWithRot(wf3d_Cylinder* cylinder, owl_v3f32 v_pos, owl_q32 q_rot)
{
    return fmaxf(cylinder->r, cylinder->half_h);
}

//Rasterization function
//
wf3d_error wf3d_Cylinder_Rasterization(wf3d_Cylinder const* cylinder, wf3d_Image2d* img_out, wf3d_lightsource const* lightsource_list, unsigned int nb_lightsources, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam)
{
    wf3d_error error = WF3D_SUCCESS;

    error = wf3d_quadratic_curve_Rasterization(&cylinder->side, img_out, lightsource_list, nb_lightsources, v_pos, q_rot, cam);
    if(error == WF3D_SUCCESS)
    {
        error = wf3d_quadratic_curve_Rasterization(&cylinder->extrem, img_out, lightsource_list, nb_lightsources, v_pos, q_rot, cam);
    }

    return error;
}
