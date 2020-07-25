#ifndef WF3D_RASTERIZATION_ATTR_H_INCLUDED
#define WF3D_RASTERIZATION_ATTR_H_INCLUDED

#include <WF3D/Rendering/camera3d.h>

typedef struct
{
    int width;
    int height;

    int x_min;
    int x_max;
    int y_min;
    int y_max;

} wf3d_rasterization_rectangle;

typedef struct
{
    //Custom argument for callback
    void const* callback_arg;

    void (*callback_fct)(wf3d_rasterization_rectangle const* rect, int x, int y, void const* callback_arg, owl_v3f32 v_intersection, owl_v3f32 normal);

} wf3d_rasterization_callback;

#endif // WF3D_RASTERIZATION_ATTR_H_INCLUDED
