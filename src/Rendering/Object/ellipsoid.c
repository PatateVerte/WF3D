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
float wf3d_Ellipsoid_Radius(wf3d_Ellipsoid* ellipsoid)
{
    return fmaxf(ellipsoid->r[2], fmaxf(ellipsoid->r[1], ellipsoid->r[0]));
}

//
float wf3d_Ellipsoid_InfRadius(wf3d_Ellipsoid* ellipsoid, owl_v3f32 v_pos)
{
    return fmaxf(ellipsoid->r[2], fmaxf(ellipsoid->r[1], ellipsoid->r[0]));
}

//
float wf3d_Ellipsoid_InfRadiusWithRot(wf3d_Ellipsoid* ellipsoid, owl_v3f32 v_pos, owl_q32 q_rot)
{
    return fmaxf(ellipsoid->r[2], fmaxf(ellipsoid->r[1], ellipsoid->r[0]));
}

//Rasterization function
//
wf3d_error wf3d_Ellipsoid_Rasterization(wf3d_Ellipsoid const* ellipsoid, wf3d_Image2d* img_out, wf3d_lightsource const* lightsource_list, unsigned int nb_lightsources, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam)
{
    return wf3d_quadratic_curve_Rasterization(&ellipsoid->curve, img_out, lightsource_list, nb_lightsources, v_pos, q_rot, cam);
}
