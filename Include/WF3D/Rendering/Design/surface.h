#ifndef WF3D_SURFACE_H_INCLUDED
#define WF3D_SURFACE_H_INCLUDED

#include <stdbool.h>

#include <OWL/owl.h>

#include <WF3D/Rendering/Design/color.h>

typedef enum
{
    WF3D_DEFAULT_SHADING,
    WF3D_PHONG_SHADING

} wf3d_shading_model;

typedef struct
{
    wf3d_color reflection_color;
    wf3d_color refraction_color;

    //If set to true, the surface will be ignored during Rasterization
    //This field has no impact on ray tracing
    bool transparent;

    //Relative Refractive index = n1 / n2
    //Where n1 is the refractive index in the direction where the normal points to
    //Where n2 is the refractive index in the direction behind the normal
    float rel_refractive_index;

    wf3d_shading_model shading_model;

    //Phong lightning data
    float shininess;
    wf3d_color specular_color;

} wf3d_surface;

#endif // WF3D_SURFACE_H_INCLUDED
