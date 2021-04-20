#ifndef WF3D_QUADRATIC_CURVE_H_INCLUDED
#define WF3D_QUADRATIC_CURVE_H_INCLUDED

#include <stdbool.h>

#include <OWL/owl.h>

#include <WF3D/error.h>

#include <OWL/Optimized3d/vector/v3f32.h>
#include <OWL/Optimized3d/quaternion/q32.h>

#include <WF3D/Rendering/camera3d.h>
#include <WF3D/Rendering/Shapes/rasterization_attr.h>
#include <WF3D/Rendering/Shapes/triangle3d.h>

typedef struct OWL_ALIGN16
{
    //The product * of 2 vectors means the product component by component
    //For example (x y z) * (x_ y_ z_) = (x*x_ y*y_ z*z_)

    //Must send the canonical base to the eigenbasis
    owl_q32 q_eigenbasis;

    //Spatial delimitation
    owl_v3f32 norminf_filter;
    owl_v3f32 norm2_filter;

    /*
        The curve in limited for the points h = (x y z) (expressed in the eigenbasis) for whom :
        ||norminf_filter * h||_inf <= 1.0
        ||norm2_filter * h||_2 <= 1.0

        For example, norminf_filter = norm2_filter = 0 describes an infinite space
    */

    //Describes the curve
    owl_v3f32 a;    // = (a1 a2 a3)
    owl_v3f32 alpha;    // = (alpha1 alpha2 alpha3)
    float c;

    /*
        The curve is described by the equation :
            c + a.h + alpha.(h*h)
        with h=(x y z) the coordinates of a point expressed in the eigenbasis
    */

} wf3d_quadratic_curve;

//Set up of a quadratic curve
OWL_DLL_EXPORT wf3d_quadratic_curve* wf3d_quadratic_curve_set(wf3d_quadratic_curve* curve, owl_q32 q_eigenbasis, owl_v3f32 norminf_filter, owl_v3f32 norm2_filter, float c, owl_v3f32 a, owl_v3f32 alpha);

//The intersection between a ray and the quadratic curve
//Return true if the intersection exists and returns the parameter, false otherwise
//Optional parameters (modified only if an intersection has been found) :
//t to return the parameter for the nearest intersection (v_intersection = ray_origin + t*ray_dir)
//normal_ret to return the normal of the intersection
OWL_DLL_EXPORT bool OWL_VECTORCALL wf3d_quadratic_curve_NearestIntersectionWithRay(wf3d_quadratic_curve const* curve, owl_v3f32 v_pos, owl_q32 q_rot, owl_v3f32 ray_origin, owl_v3f32 ray_dir, float t_min, float t_max, float* t_ret, owl_v3f32* normal_ret);

OWL_DLL_EXPORT void OWL_VECTORCALL wf3d_quadratic_curve_Rasterization(wf3d_quadratic_curve const* curve, wf3d_rasterization_callback const* callback, wf3d_rasterization_rectangle const* rect, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam);

#endif // WF3D_QUADRATIC_CURVE_H_INCLUDED
