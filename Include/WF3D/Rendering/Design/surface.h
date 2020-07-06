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
			float relative_refractive_index;    // = n1 / n2
                                                //n1 refractive index before interaction with the surface
                                                //n2 refractive index after interaction with the surface
		};
	};

} wf3d_surface;

wf3d_surface* wf3d_surface_mix(wf3d_surface* mixed_surface, wf3d_surface const* const* surface_list, float const* coeff, int nb_colors);

#endif // WF3D_SURFACE_H_INCLUDED
