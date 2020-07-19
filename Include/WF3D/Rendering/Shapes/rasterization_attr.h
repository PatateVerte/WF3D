#ifndef WF3D_RASTERIZATION_ATTR_H_INCLUDED
#define WF3D_RASTERIZATION_ATTR_H_INCLUDED

#include <WF3D/Rendering/camera3d.h>
#include <WF3D/Rendering/lightsource.h>

typedef struct
{
    wf3d_camera3d const* cam;

    wf3d_lightsource const* const* lightsource_list;
    unsigned int nb_lightsources;

} wf3d_rasterization_env;

#endif // WF3D_RASTERIZATION_ATTR_H_INCLUDED
