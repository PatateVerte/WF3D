#ifndef WF3D_CYLINDER_H_INCLUDED
#define WF3D_CYLINDER_H_INCLUDED

#include <stdbool.h>

#include <OWL/Optimized3d/vector/v3f32.h>

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
float wf3d_Cylinder_Radius(wf3d_Cylinder const* cylinder);

//
float wf3d_Cylinder_InfRadius(wf3d_Cylinder const* cylinder, owl_v3f32 v_pos);

//
float wf3d_Cylinder_InfRadiusWithRot(wf3d_Cylinder const* cylinder, owl_v3f32 v_pos, owl_q32 q_rot);

//The intersection between a ray and the cylinder
//Return true if the intersection exists and returns the parameter, false otherwise
//Optional parameters (modified only if an intersection has been found) :
//t to return the parameter for the nearest intersection (v_intersection = ray_origin + t*ray_dir)
//normal_ret to return the normal of the intersection
//surface_ret to return the surface of the intersection
bool wf3d_Cylinder_NearestIntersectionWithRay(wf3d_Cylinder const* cylinder, owl_v3f32 v_pos, owl_q32 q_rot, owl_v3f32 ray_origin, owl_v3f32 ray_dir, float t_min, float t_max, float* t_ret, owl_v3f32* normal_ret, wf3d_surface* surface_ret);

//Rasterization function
wf3d_error wf3d_Cylinder_Rasterization(wf3d_Cylinder const* cylinder, wf3d_image2d_rectangle* img_out, wf3d_rasterization_env const* env, owl_v3f32 v_pos, owl_q32 q_rot);

//Rasterization2 function
wf3d_error wf3d_Cylinder_Rasterization2(wf3d_Cylinder const* cylinder, wf3d_image3d_image_piece* img_out, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam);



#endif // WF3D_CYLINDER_H_INCLUDED
