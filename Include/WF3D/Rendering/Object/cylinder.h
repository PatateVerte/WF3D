#ifndef WF3D_CYLINDER_H_INCLUDED
#define WF3D_CYLINDER_H_INCLUDED

#include <stdbool.h>

#include <OWL/v3f32.h>

#include <WF3D/error.h>
#include <WF3D/Rendering/camera3d.h>
#include <WF3D/Rendering/Shapes/quadratic_curve.h>
#include <WF3D/Rendering/Design/image3d.h>

typedef struct
{
    wf3d_quadratic_curve side;
    wf3d_quadratic_curve extrem;

    float r;
    float h;

} wf3d_Cylinder;

//Create an cylinder with x as rotation axis
wf3d_Cylinder* wf3d_Cylinder_Create(float r, float h, wf3d_surface const* surface_side, wf3d_surface const* surface_extrem);

//
wf3d_Cylinder* wf3d_Cylinder_UpdateRadiusAndHeight(wf3d_Cylinder* cylinder, float r, float h);

//
wf3d_Cylinder* wf3d_Cylinder_UpdateRadius(wf3d_Cylinder* cylinder, float r);

//
wf3d_Cylinder* wf3d_Cylinder_UpdateHeight(wf3d_Cylinder* cylinder, float h);

//
void wf3d_Cylinder_Destroy(wf3d_Cylinder* cylinder);

//
float wf3d_Cylinder_Radius(wf3d_Cylinder* cylinder);

//
float wf3d_Cylinder_InfRadius(wf3d_Cylinder* cylinder, owl_v3f32 v_pos);

//
float wf3d_Cylinder_InfRadiusWithRot(wf3d_Cylinder* cylinder, owl_v3f32 v_pos, owl_q32 q_rot);

//Rasterization function
//
wf3d_error wf3d_Cylinder_Rasterization(wf3d_Cylinder const* cylinder, wf3d_Image2d* img_out, wf3d_lightsource const* lightsource_list, unsigned int nb_lightsources, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam);



#endif // WF3D_CYLINDER_H_INCLUDED
