#ifndef WF3D_SURFACE_H_INCLUDED
#define WF3D_SURFACE_H_INCLUDED

#include <stdbool.h>

#include <OWL/owl.h>

typedef enum
{
    WF3D_DEFAULT_SHADING,
    WF3D_PHONG_SHADING

} wf3d_shading_model;

typedef struct
{
    float reflection_filter[4] OWL_ALIGN16;
    float refraction_filter[4] OWL_ALIGN16;

    //If set to true, the surface will be ignored during Rasterization
    //This field has no impact on ray tracing
    bool transparent;

    //Relative Refractive index = n1 / n2
    //Where n1 is the refractive index in the direction where the normal points to
    //Where n2 is the refractive index in the direction behind where the normal points to
    float rel_refractive_index;

    wf3d_shading_model shading_model;

    //Phong lightning data
    float shininess;
    float specular_filter[4] OWL_ALIGN16;

} wf3d_surface;

#endif // WF3D_SURFACE_H_INCLUDED
