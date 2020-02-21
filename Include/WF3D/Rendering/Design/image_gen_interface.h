#ifndef WF3D_IMAGE_GEN_INTERFACE_H_INCLUDED
#define WF3D_IMAGE_GEN_INTERFACE_H_INCLUDED

#include <WF3D/Rendering/Design/color.h>

typedef struct
{
    int width;
    int height;

    //int set_pixel_callback(void* img_obj, int x, int y, wf3d_color const* color)
    //Return non-zero if failed
    int (*set_pixel_callback)(void*, int, int, wf3d_color const*);

    //int get_pixel_callback(void* img_obj, int x, int y, wf3d_color const* color)
    //Return non-zero if failed
    int (*get_pixel_callback)(void*, int, int, wf3d_color const*);

    //
    void* img_obj;

} wf3d_img_gen_interface;

#endif // WF3D_IMAGE_GEN_INTERFACE_H_INCLUDED
