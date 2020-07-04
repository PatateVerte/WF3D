#ifndef WF3D_COLOR_H_INCLUDED
#define WF3D_COLOR_H_INCLUDED

#include <stdint.h>

#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>

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

#endif // WF3D_COLOR_H_INCLUDED
