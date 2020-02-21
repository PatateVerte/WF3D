#ifndef WF3D_CUBE_H_INCLUDED
#define WF3D_CUBE_H_INCLUDED

#include <stdbool.h>

#include <WF3D/error.h>
#include <WF3D/Geometry/vect3d.h>
#include <WF3D/Rendering/camera3d.h>
#include <WF3D/Rendering/Shapes/triangle3d.h>
#include <WF3D/Rendering/Design/image3d.h>

typedef struct
{
    wf3d_color color_list[6];
    float side;

} wf3d_ColoredCube;

//Create a colored cube
//color_list[6]
wf3d_ColoredCube* wf3d_ColoredCube_Create(float side, wf3d_color const* color_list);

//
void wf3d_ColoredCube_Destroy(wf3d_ColoredCube* cube);

//
float wf3d_ColoredCube_Radius(wf3d_ColoredCube* cube);

//
float wf3d_ColoredCube_InfRadius(wf3d_ColoredCube* cube, wf3d_vect3d v_pos);

//
float wf3d_ColoredCube_InfRadiusWithRot(wf3d_ColoredCube* cube, wf3d_vect3d v_pos, wf3d_quat q_rot);

//Rasterization function
//
wf3d_error wf3d_ColoredCube_Rasterization(wf3d_ColoredCube const* cube, wf3d_Image3d* img_out, wf3d_vect3d v_pos, wf3d_quat q_rot, wf3d_camera3d const* cam);

#endif // WF3D_CUBE_H_INCLUDED