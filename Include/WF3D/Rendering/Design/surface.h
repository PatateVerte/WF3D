#ifndef WF3D_SURFACE_H_INCLUDED
#define WF3D_SURFACE_H_INCLUDED

#include <WF3D/Rendering/Design/color.h>

typedef struct
{
    wf3d_color color;

	union
	{
		float raw_surface_data[4];

		struct {
			float diffusion;
			float reflection;
			float transparency;
		};
	};

} wf3d_surface;

wf3d_surface* wf3d_surface_mix(wf3d_surface* mixed_surface, wf3d_surface const* surface_list, float const* coeff, int nb_colors);

#endif // WF3D_SURFACE_H_INCLUDED
