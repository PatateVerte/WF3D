#ifndef WF3D_LIGHTSOURCE_H_INCLUDED
#define WF3D_LIGHTSOURCE_H_INCLUDED

#include <OWL/v3f32.h>

#include <WF3D/Rendering/Design/color.h>

typedef enum
{
    WF3D_LIGHTSOURCE_AMBIENT,
    WF3D_LIGHTSOURCE_DIRECTIONAL,
    WF3D_LIGHTSOURCE_PUNCTUAL

} wf3d_lightsource_type;

typedef struct
{
    float intensity;

} wf3d_lightsource_ambient;

typedef struct
{
    owl_v3f32 v_dir;
    float intensity;

} wf3d_lightsource_directional;

typedef struct
{
    //Position of the source
    owl_v3f32 v_pos;

    //Intensity when the distance to the source is 1
    float intensity_per_surface;

} wf3d_lightsource_punctual;

typedef struct
{
    wf3d_lightsource_type type;
    wf3d_color color;

    union
    {
        wf3d_lightsource_ambient ambient_src;
        wf3d_lightsource_directional directional_src;
        wf3d_lightsource_punctual punctual_src;
    };

} wf3d_lightsource;

wf3d_color* wf3d_lightsource_enlight(wf3d_lightsource const* lightsource, wf3d_color* dst, wf3d_color const* src, owl_v3f32 v_pos, owl_v3f32 normal);

#endif // WF3D_LIGHTSOURCE_H_INCLUDED
