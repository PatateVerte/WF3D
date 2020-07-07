#ifndef WF3D_SURFACE_H_INCLUDED
#define WF3D_SURFACE_H_INCLUDED

#include <WF3D/Rendering/Design/color.h>

typedef struct
{
    wf3d_color diffusion_color;
    float reflection_filter[4];
    float refraction_filter[4];

} wf3d_surface;

wf3d_surface* wf3d_surface_mix(wf3d_surface* mixed_surface, wf3d_surface const* const* surface_list, float const* coeff, int nb_colors);

#endif // WF3D_SURFACE_H_INCLUDED
