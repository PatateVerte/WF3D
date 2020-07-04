#ifndef WF3D_IMAGE3D_H_INCLUDED
#define WF3D_IMAGE3D_H_INCLUDED

#include <WF3D/error.h>
#include <WF3D/Rendering/Design/color.h>

#include <OWL/v3f32.h>

#include <stdio.h>
#include <stdbool.h>
#include <WF3D/Rendering/Design/image_gen_interface.h>
#include <WF3D/Rendering/lightsource.h>

typedef struct
{
    int width;
    int height;

    wf3d_color* color;
    float* z_buffer;
    owl_v3f32* M;
    owl_v3f32* normal;

} wf3d_Image3d;

//
wf3d_Image3d* wf3d_Image3d_Create(int width, int height);

//
void wf3d_Image3d_Destroy(wf3d_Image3d* img);

//
wf3d_Image3d* wf3d_Image3d_Clear(wf3d_Image3d* img, wf3d_color const* background_color);

static inline size_t wf3d_Image3d_pixel_index(wf3d_Image3d const* img, int x, int y)
{
    return (size_t)y * (size_t)img->width + (size_t)x;
}

//
static inline float wf3d_Image3d_unsafe_Depth(wf3d_Image3d const* img, int x, int y)
{
    size_t pixel_index = wf3d_Image3d_pixel_index(img, x, y);
    return img->z_buffer[pixel_index];
}

static inline wf3d_error wf3d_Image3d_SetPixel(wf3d_Image3d* img, int x, int y, wf3d_color const* color, float z, owl_v3f32 M, owl_v3f32 normal)
{
    wf3d_error error = WF3D_SUCCESS;

    if(img != NULL)
    {
        if(x >= 0 && x < img->width && y >= 0 && y < img->height)
        {
            size_t pixel_index = wf3d_Image3d_pixel_index(img, x, y);
            if(z <= img->z_buffer[pixel_index])
            {
                img->color[pixel_index] = *color;
                img->z_buffer[pixel_index] = z;
                img->M[pixel_index] = M;
                img->normal[pixel_index] = normal;
            }
        }
        else
        {
            error = WF3D_IMAGE_ACCESS_ERROR;
        }
    }

    return error;
}

//
wf3d_error wf3d_Image3d_WriteInImgGen(wf3d_Image3d const* img, wf3d_img_gen_interface* img_out);

//Adds the enlighted image3d to img_gen_interface
wf3d_error wf3d_Image3d_EnlightInImgGen(wf3d_Image3d const* img, wf3d_lightsource const* lightsource, wf3d_img_gen_interface* img_out);

//Adds the enlighted image3d to img_out
wf3d_error wf3d_Image3d_EnlightInImg3d(wf3d_Image3d const* img, wf3d_lightsource const* lightsource, wf3d_Image3d* img_out);

//
int wf3d_Image3d_WriteInBMPFile(wf3d_Image3d const* img, FILE* bmp_file);

#endif // IMAGE3D_H_INCLUDED
