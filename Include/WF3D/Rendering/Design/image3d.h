#ifndef WF3D_IMAGE3D_H_INCLUDED
#define WF3D_IMAGE3D_H_INCLUDED

#include <WF3D/error.h>
#include <WF3D/Rendering/Design/surface.h>

#include <OWL/Optimized3d/vector/v3f32.h>

#include <stdio.h>
#include <stdbool.h>
#include <WF3D/Rendering/Design/image_gen_interface.h>
#include <WF3D/Rendering/lightsource.h>

typedef struct
{
    int width;
    int height;

    wf3d_surface* surface;
    float* depth_buffer;
    owl_v3f32* M;
    owl_v3f32* normal;

} wf3d_Image3d;

//
wf3d_Image3d* wf3d_Image3d_Create(int width, int height);

//
void wf3d_Image3d_Destroy(wf3d_Image3d* img);

//
wf3d_Image3d* wf3d_Image3d_Clear(wf3d_Image3d* img);

static inline size_t wf3d_Image3d_pixel_index(wf3d_Image3d const* img, int x, int y)
{
    return (size_t)y * (size_t)img->width + (size_t)x;
}

//
static inline float wf3d_Image3d_unsafe_Depth(wf3d_Image3d const* img, int x, int y)
{
    size_t pixel_index = wf3d_Image3d_pixel_index(img, x, y);
    return img->depth_buffer[pixel_index];
}

//
static inline wf3d_surface const* wf3d_Image3d_unsafe_Surface(wf3d_Image3d const* img, int x, int y)
{
    size_t pixel_index = wf3d_Image3d_pixel_index(img, x, y);
    return img->surface + pixel_index;
}

//
static inline owl_v3f32 wf3d_Image3d_unsafe_M(wf3d_Image3d const* img, int x, int y)
{
    size_t pixel_index = wf3d_Image3d_pixel_index(img, x, y);
    return img->M[pixel_index];
}

//
static inline owl_v3f32 wf3d_Image3d_unsafe_Normal(wf3d_Image3d const* img, int x, int y)
{
    size_t pixel_index = wf3d_Image3d_pixel_index(img, x, y);
    return img->normal[pixel_index];
}

static inline wf3d_error wf3d_Image3d_SetPixel(wf3d_Image3d* img, int x, int y, wf3d_surface const* surface, float depth, owl_v3f32 M, owl_v3f32 normal)
{
    wf3d_error error = WF3D_SUCCESS;

    if(x >= 0 && x < img->width && y >= 0 && y < img->height)
    {
        size_t pixel_index = wf3d_Image3d_pixel_index(img, x, y);

        img->surface[pixel_index] = *surface;
        img->depth_buffer[pixel_index] = depth;
        img->M[pixel_index] = M;
        img->normal[pixel_index] = normal;
    }
    else
    {
        error = WF3D_IMAGE_ACCESS_ERROR;
    }

    return error;
}

//
typedef struct
{
    wf3d_Image3d* img3d;

    int full_img_width;
    int full_img_height;

    int x_min;
    int x_max;
    int y_min;
    int y_max;

} wf3d_image3d_image_piece;

#endif // IMAGE3D_H_INCLUDED
