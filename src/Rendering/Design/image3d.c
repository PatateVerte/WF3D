#include <WF3D/Rendering/Design/image3d.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <malloc.h>

//
//
//
wf3d_Image3d* wf3d_Image3d_Create(int width, int height)
{
    wf3d_Image3d* img = malloc(sizeof(*img));
    size_t nb_pixel = (size_t)width * (size_t)height;

    if(img != NULL)
    {
        img->width = width;
        img->height = height;

        img->color = malloc(nb_pixel * sizeof(*(img->color)));
        img->z_buffer = malloc(nb_pixel * sizeof(*(img->z_buffer)));
        img->M = _aligned_malloc(nb_pixel * sizeof(*(img->M)), 16);
        img->normal = _aligned_malloc(nb_pixel * sizeof(*(img->normal)), 16);

        if(nb_pixel > 0 && (img->color == NULL || img->z_buffer == NULL || img->M == NULL || img->normal == NULL))
        {
            wf3d_Image3d_Destroy(img);
            img = NULL;
        }
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
        free(img->color);
        free(img->z_buffer);
        _aligned_free(img->M);
        _aligned_free(img->normal);

        free(img);
    }
}

//
//
//
wf3d_Image3d* wf3d_Image3d_Clear(wf3d_Image3d* img, wf3d_color const* background_color)
{
    if(img != NULL)
    {
        size_t nb_pixels = (size_t)img->width *  (size_t)img->height;

        for(size_t k = 0 ; k < nb_pixels ; k++)
        {
            img->color[k] = *background_color;
            img->z_buffer[k] = INFINITY;
        }
    }

    return img;
}

//
//
//
/*wf3d_error wf3d_Image3d_SetPixel(wf3d_Image3d* img, int x, int y, wf3d_color const* color, float z, wf3d_vect3d M, wf3d_vect3d normal)
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
}*/

//
//
//
wf3d_error wf3d_Image3d_WriteInImgGen(wf3d_Image3d const* img, wf3d_img_gen_interface* img_out)
{
    wf3d_error error = WF3D_SUCCESS;
    int access_error = 0;

    if(img != NULL)
    {
        for(int x = 0 ; x < img->width && error == WF3D_SUCCESS ; x++)
        {
            for(int y = 0 ; y < img->height ; y++)
            {
                size_t pixel_index = wf3d_Image3d_pixel_index(img, x, y);
                access_error = img_out->set_pixel_callback(img_out->img_obj, x, y, img->color + pixel_index);
                if(access_error != 0)
                {
                    error = WF3D_IMAGE_ACCESS_ERROR;
                }
            }
        }
    }

    return error;
}

//Adds the enlighted image3d to img_gen_interface
//
//
wf3d_error wf3d_Image3d_EnlightInImgGen(wf3d_Image3d const* img, wf3d_lightsource const* lightsource, wf3d_img_gen_interface* img_out)
{
    wf3d_error error = WF3D_SUCCESS;
    int access_error = 0;

    if(img != NULL)
    {
        for(int x = 0 ; x < img->width && error == WF3D_SUCCESS ; x++)
        {
            for(int y = 0 ; y < img->height ; y++)
            {
                size_t pixel_index = wf3d_Image3d_pixel_index(img, x, y);

                wf3d_color pixel_color;
                wf3d_lightsource_enlight(lightsource, &pixel_color, img->color + pixel_index, img->M[pixel_index], img->normal[pixel_index]);

                wf3d_color previous_color;
                access_error = img_out->get_pixel_callback(img_out->img_obj, x, y, &previous_color);

                if(access_error == 0)
                {
                    wf3d_color final_color;
                    for(unsigned int i = 0 ; i < 3 ; i++)
                    {
                        final_color.rgba[i] = pixel_color.rgba[i] + previous_color.rgba[i];
                    }
                    final_color.rgba[3] = pixel_color.rgba[3];

                    access_error = img_out->set_pixel_callback(img_out->img_obj, x, y, &final_color);
                }
                else
                {
                    error = WF3D_IMAGE_ACCESS_ERROR;
                }
            }
        }
    }

    return error;
}

//Adds the enlighted image3d to img_out
//
//
wf3d_error wf3d_Image3d_EnlightInImg3d(wf3d_Image3d const* img, wf3d_lightsource const* lightsource, wf3d_Image3d* img_out)
{
    wf3d_error error = WF3D_SUCCESS;

    if(img != NULL)
    {
        if(img->width == img_out->width && img->height == img_out->height)
        {
            for(int x = 0 ; x < img->width && error == WF3D_SUCCESS ; x++)
            {
                for(int y = 0 ; y < img->height ; y++)
                {
                    size_t pixel_index = wf3d_Image3d_pixel_index(img, x, y);

                    wf3d_color pixel_color;
                    wf3d_lightsource_enlight(lightsource, &pixel_color, img->color + pixel_index, img->M[pixel_index], img->normal[pixel_index]);

                    wf3d_color final_color;
                    for(unsigned int i = 0 ; i < 3 ; i++)
                    {
                        final_color.rgba[i] = pixel_color.rgba[i] + img_out->color[pixel_index].rgba[i];
                    }
                    final_color.rgba[3] = pixel_color.rgba[3];

                    img_out->color[pixel_index] = final_color;
                }
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
                uint8_t shuffled_buffer[4];
                wf3d_color_GetRGBA(img->color + k, shuffled_buffer);

                uint8_t buff[4];
                for(int c = 0 ; c < 3 ; c++)
                {
                    buff[c] = shuffled_buffer[2 - c];
                }
                buff[3] = shuffled_buffer[3];

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
