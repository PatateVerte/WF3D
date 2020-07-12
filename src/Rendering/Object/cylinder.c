#include <WF3D/Rendering/Object/cylinder.h>

//Create an cylinder
//
//
wf3d_Cylinder* wf3d_Cylinder_Create(float r, float h, wf3d_surface const* surface_side, wf3d_surface const* surface_extrem)
{
    wf3d_Cylinder* cylinder = malloc(sizeof(*cylinder));

    if(cylinder != NULL)
    {
        wf3d_quadratic_curve_set_design(&cylinder->side, surface_side);
        wf3d_quadratic_curve_set_design(&cylinder->extrem, surface_extrem);

        wf3d_Cylinder_UpdateRadiusAndHeight(cylinder, r, h);
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
wf3d_Cylinder* wf3d_Cylinder_UpdateRadiusAndHeight(wf3d_Cylinder* cylinder, float r, float h)
{
    float half_h = 0.5 * h;

    cylinder->r = r;
    cylinder->h = h;

    float const delta = 1.0 - (1.0 / ((float)(1<<7)));
    owl_q32 q_eigenbasis = owl_q32_from_real(1.0);

    owl_v3f32 norminf_filter_side = owl_v3f32_set(1.0 / half_h, delta / r, delta / r);
    owl_v3f32 norm2_filter_side = owl_v3f32_broadcast(delta / sqrtf(r*r + half_h*half_h));
    float c_side = - r*r;
    owl_v3f32 a_side = owl_v3f32_zero();
    owl_v3f32 alpha_side = owl_v3f32_set(0.0, 1.0, 1.0);
    wf3d_quadratic_curve_set_geometry(&cylinder->side, q_eigenbasis, norminf_filter_side, norm2_filter_side, c_side, a_side, alpha_side);

    owl_v3f32 norminf_filter_extrem = owl_v3f32_set(delta / half_h, delta / r, delta / r);
    owl_v3f32 norm2_filter_extrem = owl_v3f32_set(0.0, 1.0 / r, 1.0 / r);
    float c_extrem = - half_h * half_h;
    owl_v3f32 a_extrem = owl_v3f32_zero();
    owl_v3f32 alpha_extrem = owl_v3f32_set(1.0, 0.0, 0.0);
    wf3d_quadratic_curve_set_geometry(&cylinder->extrem, q_eigenbasis, norminf_filter_extrem, norm2_filter_extrem, c_extrem, a_extrem, alpha_extrem);

    return cylinder;
}

//
wf3d_Cylinder* wf3d_Cylinder_UpdateRadius(wf3d_Cylinder* cylinder, float r)
{
    return wf3d_Cylinder_UpdateRadiusAndHeight(cylinder, r, cylinder->h);
}

//
wf3d_Cylinder* wf3d_Cylinder_UpdateHeight(wf3d_Cylinder* cylinder, float h)
{
    return wf3d_Cylinder_UpdateRadiusAndHeight(cylinder, cylinder->r, h);
}

//
//
//
float wf3d_Cylinder_Radius(wf3d_Cylinder const* cylinder)
{
    return sqrtf(cylinder->r * cylinder->r + 0.25 * cylinder->h * cylinder->h);
}

//
//
//
float wf3d_Cylinder_InfRadius(wf3d_Cylinder const* cylinder, owl_v3f32 v_pos)
{
    owl_v3f32 geom_broadcast = owl_v3f32_set(0.5 * cylinder->h, cylinder->r, cylinder->h);
    return fmaxf(
                    owl_v3f32_norminf(owl_v3f32_add(v_pos, geom_broadcast)),
                    owl_v3f32_norminf(owl_v3f32_sub(v_pos, geom_broadcast))
                 );
}

//
float wf3d_Cylinder_InfRadiusWithRot(wf3d_Cylinder const* cylinder, owl_v3f32 v_pos, owl_q32 q_rot)
{
    owl_v3f32 base_xyz[3];
    owl_v3f32_setbase_xyz(base_xyz, 0.5 * cylinder->h, cylinder->r, cylinder->r);

    for(unsigned int j = 0 ; j < 3 ; j++)
    {
        base_xyz[j] = owl_q32_transform_v3f32(q_rot, base_xyz[j]);
    }

    float inf_radius = 0.0;
    for(float sign_z = -1.0 ; sign_z <= 1.0 ; sign_z += 2.0)
    {
        owl_v3f32 center_z = owl_v3f32_add_scalar_mul(v_pos, base_xyz[2], sign_z);
        for(float sign_y = -1.0 ; sign_y <= 1.0 ; sign_y += 2.0)
        {
            owl_v3f32 center_y = owl_v3f32_add_scalar_mul(center_z, base_xyz[1], sign_y);
            for(float sign_x = -1.0 ; sign_x <= 1.0 ; sign_x += 2.0)
            {
                owl_v3f32 vertex = owl_v3f32_add_scalar_mul(center_y, base_xyz[0], sign_x);
                inf_radius = fmaxf(inf_radius, owl_v3f32_norminf(vertex));
            }
        }
    }

    return inf_radius;
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
