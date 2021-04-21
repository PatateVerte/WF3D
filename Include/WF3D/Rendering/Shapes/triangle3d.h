#ifndef WF3D_TRIANGLE_H_INCLUDED
#define WF3D_TRIANGLE_H_INCLUDED

#include <OWL/Optimized3d/vector/v3f32.h>
#include <OWL/Optimized3d/quaternion/q32.h>

#include <WF3D/error.h>

#include <WF3D/Rendering/camera3d.h>
#include <WF3D/Rendering/Shapes/rasterization_attr.h>

#include <stdbool.h>

typedef struct
{
    owl_v3f32 vertex_list[3];
    owl_v3f32 normal;

} wf3d_triangle3d;

WF3D_DLL_EXPORT wf3d_triangle3d* wf3d_triangle3d_Set(wf3d_triangle3d* triangle, owl_v3f32* vertex_list, owl_v3f32 normal);

//
WF3D_DLL_EXPORT wf3d_triangle3d* wf3d_triangle3d_ComputeNormal(wf3d_triangle3d* triangle);

//Move triangle
WF3D_DLL_EXPORT wf3d_triangle3d* OWL_VECTORCALL wf3d_triangle3d_Move(wf3d_triangle3d* t_dst, wf3d_triangle3d const* t_src, owl_v3f32 v);

//Transform triangle (only the coordinates)
WF3D_DLL_EXPORT wf3d_triangle3d* OWL_VECTORCALL wf3d_triangle3d_Transform(wf3d_triangle3d* t_dst, wf3d_triangle3d const* t_src, owl_v3f32 v, owl_q32 q_rot);

//The intersection between a ray and the triangle
//Return true if the intersection exists and returns the parameter, false otherwise
//Optional parameters (modified only if an intersection has been found) :
//t to return the parameter for the nearest intersection (v_intersection = ray_origin + t*ray_dir)
//normal_ret to return the normal of the intersection
WF3D_DLL_EXPORT bool OWL_VECTORCALL wf3d_triangle3d_NearestIntersectionWithRay(wf3d_triangle3d const* triangle, owl_v3f32 v_pos, owl_q32 q_rot, owl_v3f32 ray_origin, owl_v3f32 ray_dir, float t_min, float t_max, float* t_ret, owl_v3f32* normal_ret);

//Rasterization with custom callback function
WF3D_DLL_EXPORT void OWL_VECTORCALL wf3d_triangle3d_Rasterization(wf3d_triangle3d const* triangle, wf3d_rasterization_callback const* callback, wf3d_rasterization_rectangle const* rect, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam);

//
WF3D_DLL_EXPORT unsigned int wf3d_triangle3d_Clipping(wf3d_triangle3d* clipped_triangle, wf3d_triangle3d const* triangle, wf3d_camera3d const* cam);

//
WF3D_DLL_EXPORT void wf3d_triangle3d_RasterizationAfterClipping(wf3d_triangle3d const* triangle, wf3d_rasterization_rectangle const* rect, wf3d_rasterization_callback const* callback, wf3d_camera3d const* cam);

#endif // WF3D_TRIANGLE_H_INCLUDED
