#include <WF3D/Rendering/Object/ellipsoid.h>

#include <math.h>

//Create an ellipsoid
//
//
wf3d_Ellipsoid* wf3d_Ellipsoid_Create(float rx, float ry, float rz, wf3d_surface const* surface)
{
    wf3d_Ellipsoid* ellipsoid = malloc(sizeof(*ellipsoid));

    if(ellipsoid != NULL)
    {
        wf3d_quadratic_curve_set_design(&ellipsoid->curve, surface);
        wf3d_Ellipsoid_UpdateAxes(ellipsoid, rx, ry, rz);
    }

    return ellipsoid;
}

//
void wf3d_Ellipsoid_Destroy(wf3d_Ellipsoid* ellipsoid)
{
    if(ellipsoid != NULL)
    {
        free(ellipsoid);
    }
}

//Updates the three axes
//
//
wf3d_Ellipsoid* wf3d_Ellipsoid_UpdateAxes(wf3d_Ellipsoid* ellipsoid, float rx, float ry, float rz)
{
    ellipsoid->r[0] = rx;
    ellipsoid->r[1] = ry;
    ellipsoid->r[2] = rz;

    owl_q32 q_eigenbasis = owl_q32_from_real(1.0);
    owl_v3f32 a = owl_v3f32_zero();
    owl_v3f32 alpha = owl_v3f32_set(1.0 / (rx*rx), 1.0 / (ry*ry), 1.0 / (rz*rz));

    float const delta = 1.0 - (1.0 / ((float)(1<<7)));
    owl_v3f32 norminf_filter = owl_v3f32_set(delta / rx, delta / ry, delta / rz);
    owl_v3f32 norm2_filter = owl_v3f32_set(delta / rx, delta / ry, delta / rz);

    wf3d_quadratic_curve_set_geometry(&ellipsoid->curve, q_eigenbasis, norminf_filter, norm2_filter, -1.0, a, alpha);

    return ellipsoid;
}

//Updates one radius of the ellipsoid
wf3d_Ellipsoid* wf3d_Ellipsoid_UpdateOneAxe(wf3d_Ellipsoid* ellipsoid, unsigned int axe_index, float r)
{
    if(axe_index < 3)
    {
        ellipsoid->r[axe_index] = r;
        return wf3d_Ellipsoid_UpdateAxes(ellipsoid, ellipsoid->r[0], ellipsoid->r[1], ellipsoid->r[2]);
    }
    else
    {
        return NULL;
    }
}

//
float wf3d_Ellipsoid_Radius(wf3d_Ellipsoid const* ellipsoid)
{
    return fmaxf(ellipsoid->r[2], fmaxf(ellipsoid->r[1], ellipsoid->r[0]));
}

//
float wf3d_Ellipsoid_InfRadius(wf3d_Ellipsoid const* ellipsoid, owl_v3f32 v_pos)
{
    return fmaxf(ellipsoid->r[2], fmaxf(ellipsoid->r[1], ellipsoid->r[0]));
}

//
float wf3d_Ellipsoid_InfRadiusWithRot(wf3d_Ellipsoid const* ellipsoid, owl_v3f32 v_pos, owl_q32 q_rot)
{
    owl_v3f32 base_xyz[3];
    owl_v3f32_setbase_xyz(base_xyz, ellipsoid->r[0], ellipsoid->r[1], ellipsoid->r[2]);

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
wf3d_error wf3d_Ellipsoid_Rasterization(wf3d_Ellipsoid const* ellipsoid, wf3d_Image2d* img_out, wf3d_lightsource const* lightsource_list, unsigned int nb_lightsources, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam)
{
    return wf3d_quadratic_curve_Rasterization(&ellipsoid->curve, img_out, lightsource_list, nb_lightsources, v_pos, q_rot, cam);
}
