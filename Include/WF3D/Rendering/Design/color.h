#ifndef WF3D_COLOR_H_INCLUDED
#define WF3D_COLOR_H_INCLUDED

#include <stdint.h>

typedef struct
{
    uint8_t data[4];   //RGB (in this order, one remaining byte to align

} wf3d_color;

wf3d_color* wf3d_color_mix_colors(wf3d_color* mixed_color, wf3d_color const* color_list, float const* coeff, int nb_colors);

#endif // WF3D_COLOR_H_INCLUDED
