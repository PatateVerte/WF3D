#ifndef WF3D_IMAGE3D_H_INCLUDED
#define WF3D_IMAGE3D_H_INCLUDED

#include <WF3D/Rendering/Design/color.h>

#include <stdio.h>
#include <stdbool.h>
#include <WF3D/Rendering/Design/image_gen_interface.h>

typedef struct
{
    int width;
    int height;

    wf3d_color* color_list;

    bool z_buffer_on;
    float* z_buffer;

} wf3d_Image3d;

//
wf3d_Image3d* wf3d_Image3d_Create(int width, int height, bool z_buffer_on);

//
void wf3d_Image3d_Destroy(wf3d_Image3d* img);

//
wf3d_Image3d* wf3d_Image3d_Clear(wf3d_Image3d* img, wf3d_color const* background_color, float z);

//
#define wf3d_Image3d_unsafe_pixel(img, x, y) \
    ( (img)->color_list[(y) * (img)->width +(x)] )

static inline size_t wf3d_Image3d_unsafe_pixel_index(wf3d_Image3d* img, int x, int y)
{
    return (size_t)y * (size_t)img->width + (size_t)x;
}

//
wf3d_color* wf3d_Image3d_GetPixelPtr(wf3d_Image3d* img, int x, int y);

//
wf3d_img_gen_interface* wf3d_Image3d_OpenGenInterface(wf3d_Image3d* img3d, wf3d_img_gen_interface* img_gen);

//
int wf3d_Image3d_SetPixelCallback(void* img_obj, int row, int column, wf3d_color const* color);

//Not tested
int wf3d_Image3d_GetPixelCallback(void* img_obj, int row, int column, wf3d_color* color);

//
int wf3d_Image3d_SetPixelCallbackZBuffer(void* img_obj, int row, int column, wf3d_color const* color, float z);

//
int wf3d_Image3d_WriteInBMPFile(wf3d_Image3d const* img, FILE* bmp_file);

#endif // IMAGE3D_H_INCLUDED
