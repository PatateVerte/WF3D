#ifndef WF3D_SURFACE_H_INCLUDED
#define WF3D_SURFACE_H_INCLUDED

#include <WF3D/Rendering/Design/color.h>

typedef struct
{
    wf3d_color color;
    float diffusion;
    float opacity;
    float reflection;

} wf3d_surface;

#endif // WF3D_SURFACE_H_INCLUDED
