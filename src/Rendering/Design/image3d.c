#include <WF3D/Rendering/Design/image3d.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

//
//
//
wf3d_Image3d* wf3d_Image3d_Create(int width, int height, bool z_buffer_on)
{
    bool error = false;
    wf3d_Image3d* img = NULL;

    img = malloc(sizeof(*img));
    if(img != NULL)
    {
        img->width = width;
        img->height = height;
        img->color_list = NULL;

        if(width >= 0 && height >= 0)
        {
            size_t nb_pixels = (size_t)width * (size_t)height;

            img->color_list = malloc(nb_pixels * sizeof(*img->color_list));

            img->z_buffer_on = z_buffer_on;
            if(z_buffer_on)
            {
                img->z_buffer = malloc(nb_pixels * sizeof(*img->z_buffer));
            }
            else
            {
                img->z_buffer = NULL;
            }

            if(nb_pixels > 0 && (img->color_list == NULL || (z_buffer_on && img->z_buffer == NULL)))
            {
                error = true;
            }
        }
        else
        {
            error = true;
        }
    }
    else
    {
        error = true;
    }

    if(error)
    {
        wf3d_Image3d_Destroy(img);
        img = NULL;
    }

    return img;
}

//
//
//
void wf3d_Image3d_Destroy(wf3d_Image3d* img)
{
    if(img != NULL)
    {
        free(img->color_list);
        free(img->z_buffer);
        free(img);
    }
}

//
//
//
wf3d_Image3d* wf3d_Image3d_Clear(wf3d_Image3d* img, wf3d_color const* background_color, float z)
{
    size_t nb_pixels = (size_t)img->width * (size_t)img->height;

    for(size_t k = 0 ; k < nb_pixels ; k++)
    {
        img->color_list[k] = *background_color;
    }

    if(img->z_buffer_on)
    {
        for(size_t k = 0 ; k < nb_pixels ; k++)
        {
            img->z_buffer[k] = z;
        }
    }

    return img;
}

//
//
//
wf3d_color* wf3d_Image3d_GetPixelPtr(wf3d_Image3d* img, int x, int y)
{
    if(y < img->height && y >= 0 && x < img->width && x >= 0)
    {
        return &(wf3d_Image3d_unsafe_pixel(img, x, y));
    }
    else
    {
        return NULL;
    }
}

//
//
//
wf3d_img_gen_interface* wf3d_Image3d_OpenGenInterface(wf3d_Image3d* img3d, wf3d_img_gen_interface* img_gen)
{
    img_gen->width = img3d->width;
    img_gen->height = img3d->height;
    img_gen->img_obj = (void*)img3d;
    img_gen->set_pixel_callback = &wf3d_Image3d_SetPixelCallback;
    img_gen->get_pixel_callback = &wf3d_Image3d_GetPixelCallback;
    img_gen->set_pixel_callback_z_buffer = &wf3d_Image3d_SetPixelCallbackZBuffer;

    return img_gen;
}

//
//
//
int wf3d_Image3d_SetPixelCallback(void* img_obj_ptr, int x, int y, wf3d_color const* color)
{
    wf3d_Image3d* img = img_obj_ptr;

    if(y < img->height && y >= 0 && x < img->width && x >= 0)
    {
        size_t pixel_index = wf3d_Image3d_unsafe_pixel_index(img, x, y);
        img->color_list[pixel_index] = *color;
        return 0;
    }
    else
    {
        return -1;
    }
}

//
//
//
int wf3d_Image3d_SetPixelCallbackZBuffer(void* img_obj_ptr, int x, int y, wf3d_color const* color, float z)
{
    wf3d_Image3d* img = img_obj_ptr;

    if(y < img->height && y >= 0 && x < img->width && x >= 0)
    {
        size_t pixel_index = wf3d_Image3d_unsafe_pixel_index(img, x, y);

        if(img->z_buffer_on)
        {
            if(z <= img->z_buffer[pixel_index])
            {
                img->z_buffer[pixel_index] = z;
                img->color_list[pixel_index] = *color;
            }
        }
        else
        {
            img->color_list[pixel_index] = *color;
        }

        return 0;
    }
    else
    {
        return -1;
    }
}

//
//
//
int wf3d_Image3d_GetPixelCallback(void* img_obj_ptr, int x, int y, wf3d_color* color)
{
    wf3d_Image3d* img = img_obj_ptr;

    if(y < img->height && y >= 0 && x < img->width && x >= 0)
    {
        size_t pixel_index = wf3d_Image3d_unsafe_pixel_index(img, x, y);
        *color = img->color_list[pixel_index];
        return 0;
    }
    else
    {
        return -1;
    }
}

//
//
//
int wf3d_Image3d_WriteInBMPFile(wf3d_Image3d const* img, FILE* bmp_file)
{
    #define DIB_HEADER_SIZE 40

    int error = 0;
    rewind(bmp_file);

    uint32_t file_total_size = 14 + DIB_HEADER_SIZE + 4 * (size_t)img->width * (size_t)img->height;
    uint16_t zero32[2] = {0};
    uint32_t starting_address = 14 + DIB_HEADER_SIZE;

    size_t nb_bloc_written = 0;

    //Write header
    nb_bloc_written += fwrite("BM", sizeof(char), 2, bmp_file);
    nb_bloc_written += fwrite(&file_total_size, sizeof(uint32_t), 1, bmp_file);
    nb_bloc_written += fwrite(zero32, 2, 2, bmp_file);
    nb_bloc_written += fwrite(&starting_address, sizeof(uint32_t), 1, bmp_file);

    if(nb_bloc_written == 6)
    {
        uint32_t DIB_header_size = DIB_HEADER_SIZE;
        uint32_t file_width = (uint32_t)img->width;
        uint32_t file_height = (uint32_t)img->height;
        uint16_t nb_planes = 1;
        uint16_t bpp = 32;
        uint32_t compression = 0;
        uint32_t img_size = 4 * file_width * file_height;
        int32_t h_resol = 5000;
        int32_t v_resol = 5000;
        uint32_t nb_colors = 0;
        uint32_t nb_imp_colors = 0;

        nb_bloc_written = 0;
        nb_bloc_written += fwrite(&DIB_header_size, sizeof(uint32_t), 1, bmp_file);
        nb_bloc_written += fwrite(&file_width, sizeof(file_width), 1, bmp_file);
        nb_bloc_written += fwrite(&file_height, sizeof(file_height), 1, bmp_file);
        nb_bloc_written += fwrite(&nb_planes, sizeof(nb_planes), 1, bmp_file);
        nb_bloc_written += fwrite(&bpp, sizeof(bpp), 1, bmp_file);
        nb_bloc_written += fwrite(&compression, sizeof(compression), 1, bmp_file);
        nb_bloc_written += fwrite(&img_size, sizeof(img_size), 1, bmp_file);
        nb_bloc_written += fwrite(&h_resol, sizeof(h_resol), 1, bmp_file);
        nb_bloc_written += fwrite(&v_resol, sizeof(v_resol), 1, bmp_file);
        nb_bloc_written += fwrite(&nb_colors, sizeof(nb_colors), 1, bmp_file);
        nb_bloc_written += fwrite(&nb_imp_colors, sizeof(nb_imp_colors), 1, bmp_file);

        if(nb_bloc_written == 11)
        {
            //Write data
            size_t nb_pixels = (size_t)img->width * (size_t)img->height;

            for(size_t k = 0 ; k < nb_pixels && error == 0 ; k++)
            {
                uint8_t buff[4];
                for(int c = 0 ; c < 3 ; c++)
                {
                    buff[c] = img->color_list[k].data[2 - c];
                }
                buff[3] = 255;

                nb_bloc_written = fwrite(buff, sizeof(uint8_t), 4, bmp_file);
                if(nb_bloc_written != 4)
                {
                    error = -1;
                }
            }
        }
        else
        {
            error = -1;
        }
    }
    else
    {
        error = -1;
    }

    return error;

    #undef DIB_HEADER_SIZE
}
