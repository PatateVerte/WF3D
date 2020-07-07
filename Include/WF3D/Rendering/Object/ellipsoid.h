#ifndef WF3D_ELLIPSOID_H_INCLUDED
#define WF3D_ELLIPSOID_H_INCLUDED

#include <stdbool.h>

#include <OWL/v3f32.h>

#include <WF3D/error.h>
#include <WF3D/Rendering/camera3d.h>
#include <WF3D/Rendering/Shapes/quadratic_curve.h>
#include <WF3D/Rendering/Design/image3d.h>

typedef struct
{
    wf3d_quadratic_curve curve;
    float r[3];

} wf3d_Ellipsoid;

//Create an sphere
wf3d_Ellipsoid* wf3d_Ellipsoid_Create(float rx, float ry, float rz, wf3d_surface const* surface);

//
void wf3d_Ellipsoid_Destroy(wf3d_Ellipsoid* sphere);

//
float wf3d_Ellipsoid_Radius(wf3d_Ellipsoid* sphere);

//
float wf3d_Ellipsoid_InfRadius(wf3d_Ellipsoid* sphere, owl_v3f32 v_pos);

//
float wf3d_Ellipsoid_InfRadiusWithRot(wf3d_Ellipsoid* sphere, owl_v3f32 v_pos, owl_q32 q_rot);

//Rasterization function
//
wf3d_error wf3d_Ellipsoid_Rasterization(wf3d_Ellipsoid const* sphere, wf3d_Image2d* img_out, wf3d_lightsource const* lightsource_list, unsigned int nb_lightsources, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam);



#endif // WF3D_ELLIPSOID_H_INCLUDED
