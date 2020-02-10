#ifndef WF3D_LIGHTSOURCE_H_INCLUDED
#define WF3D_LIGHTSOURCE_H_INCLUDED

#include <WF3D/Geometry/vect3d.h>
#include <WF3D/Rendering/Design/color.h>

typedef struct
{
    wf3d_vect3d v_pos;

    //Light color
    float light_color[4];

    //
    float R0;

    //Intensity when r = 0.0
    float L_0;

    //Intensity when r = +inf
    float L_inf;

} wf3d_lightsource;

//Advanced set up
wf3d_lightsource* wf3d_lightsource_set(wf3d_lightsource* ls, wf3d_vect3d v_pos, float const* light_color, float R0, float L_0, float L_inf);

//
float* wf3d_lightsource_getlightfilter(float* filter_ret, wf3d_vect3d M, wf3d_vect3d n, wf3d_lightsource const* ls_list, int nb_ls);

#endif // WF3D_LIGHTSOURCE_H_INCLUDED
