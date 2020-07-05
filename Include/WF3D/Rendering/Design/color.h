#ifndef WF3D_COLOR_H_INCLUDED
#define WF3D_COLOR_H_INCLUDED

#include <stdint.h>

#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>

#include <math.h>

typedef struct
{
    float rgba[4];

} wf3d_color;

//Set a color
wf3d_color* wf3d_color_SetRGBA(wf3d_color* color, uint8_t* rgba);

//Return rgba code in rgba[4]
uint8_t* wf3d_color_GetRGBA(wf3d_color const* color, uint8_t* rgba);

//Mix several colors
wf3d_color* wf3d_color_mix_colors(wf3d_color* mixed_color, wf3d_color const* color_list, float const* coeff, int nb_colors);

//filter = float[4]
wf3d_color* wf3d_color_filter(wf3d_color* dst, wf3d_color const* src, float* filter);

typedef struct
{
    uint8_t rgba[4];

} wf3d_color_uint8;

static inline wf3d_color_uint8* wf3d_color_uint8_from_color(wf3d_color_uint8* color8, wf3d_color const* color)
{
    for(unsigned int i = 0 ; i < 4 ; i++)
    {
        float comp = 255.0 * color->rgba[i];
        color8->rgba[i] = (uint8_t)(roundf(fmaxf(0.0, fminf(255.0, comp))));
    }

    return color8;
}

static inline wf3d_color* wf3d_color_from_color_uint8(wf3d_color* color, wf3d_color_uint8 const* color8)
{
    for(unsigned int i = 0 ; i < 4 ; i++)
    {
        color->rgba[i] = (float)color8->rgba[i] / 255.0;
    }

    return color;
}

#endif // WF3D_COLOR_H_INCLUDED
