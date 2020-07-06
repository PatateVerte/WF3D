#ifndef WF3D_QUADRATIC_CURVE_H_INCLUDED
#define WF3D_QUADRATIC_CURVE_H_INCLUDED

#include <OWL/owl.h>

#include <WF3D/error.h>

#include <OWL/v3f32.h>
#include <OWL/q32.h>

#include <WF3D/Rendering/camera3d.h>
#include <WF3D/Rendering/lightsource.h>
#include <WF3D/Rendering/Design/surface.h>
#include <WF3D/Rendering/Design/image2d.h>
#include <WF3D/Rendering/Design/image3d.h>

typedef struct
{
    //The product * of 2 vectors means the product component by component
    //For example (x y z) * (x_ y_ z_) = (xx_ yy_ zz_)

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

    wf3d_surface surface_data;

} wf3d_quadratic_curve OWL_ALIGN16;

//Set up of a quadratic curve
wf3d_quadratic_curve* wf3d_quadratic_curve_set(wf3d_quadratic_curve* curve, owl_q32 q_eigenbasis, owl_v3f32 norminf_filter, owl_v3f32 norm2_filter, float c, owl_v3f32 a, owl_v3f32 alpha, wf3d_surface const* surface_data);

//The intersection between a ray and the quadratic curve
//Return true if the intersection exists and returns the parameter, false otherwise
//t contains the parameter for the nearest intersection
bool wf3d_quadratic_curve_NearestIntersectionWithRay(wf3d_quadratic_curve const* curve, owl_v3f32 v_pos, owl_v3f32 q_rot, owl_v3f32 ray_origin, owl_v3f32 ray_dir, float t_min, float t_max, float* t);

//Rasterization of a quadratic curve
wf3d_error wf3d_quadratic_curve_Rasterization(wf3d_quadratic_curve const* curve, wf3d_Image2d* img_out, wf3d_lightsource const* lightsource_list, unsigned int nb_lightsources, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam);

#endif // WF3D_QUADRATIC_CURVE_H_INCLUDED
