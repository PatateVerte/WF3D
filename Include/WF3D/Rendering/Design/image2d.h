#ifndef WF3D_IMAGE2D_H_INCLUDED
#define WF3D_IMAGE2D_H_INCLUDED

#include <WF3D/error.h>
#include <WF3D/Rendering/Design/color.h>

#include <stdio.h>
#include <stdbool.h>
#include <WF3D/Rendering/Design/image_gen_interface.h>

typedef struct
{
    int width;
    int height;

    wf3d_color_uint8* color;
    float* depth_buffer;

} wf3d_Image2d;

//
wf3d_Image2d* wf3d_Image2d_Create(int width, int height);

//
void wf3d_Image2d_Destroy(wf3d_Image2d* img);

//
wf3d_Image2d* wf3d_Image2d_Clear(wf3d_Image2d* img, wf3d_color const* background_color);

static inline size_t wf3d_Image2d_pixel_index(wf3d_Image2d const* img, int x, int y)
{
    return (size_t)y * (size_t)img->width + (size_t)x;
}

//
static inline float wf3d_Image2d_unsafe_Depth(wf3d_Image2d const* img, int x, int y)
{
    size_t pixel_index = wf3d_Image2d_pixel_index(img, x, y);
    return img->depth_buffer[pixel_index];
}

//
static inline wf3d_error wf3d_Image2d_SetPixel(wf3d_Image2d* img, int x, int y, wf3d_color const* color, float depth)
{
    wf3d_error error = WF3D_SUCCESS;

    if(x >= 0 && x < img->width && y >= 0 && y < img->height)
    {
        size_t pixel_index = wf3d_Image2d_pixel_index(img, x, y);

        wf3d_color_uint8_from_color(img->color + pixel_index, color);
        img->depth_buffer[pixel_index] = depth;
    }
    else
    {
        error = WF3D_IMAGE_ACCESS_ERROR;
    }

    return error;
}

//Write Image2d in a bitmap file
wf3d_error wf3d_Image2d_WriteInImgGen(wf3d_Image2d const* img, wf3d_img_gen_interface* img_out);

//
int wf3d_Image2d_WriteInBMPFile(wf3d_Image2d const* img, FILE* bmp_file);

//Perform FXAA on an image
wf3d_error wf3d_Image2d_FXAA(wf3d_Image2d* img_out, wf3d_Image2d const* img_src);

//[x_min,x_max[
//[y_min,y_max[
typedef struct
{
    wf3d_Image2d* img2d;

    int x_min;
    int x_max;
    int y_min;
    int y_max;

} wf3d_image2d_rectangle;

#endif // WF3D_IMAGE2D_H_INCLUDED
