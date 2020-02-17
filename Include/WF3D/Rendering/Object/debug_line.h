#ifndef WF3D_DEBUG_LINE_H_INCLUDED
#define WF3D_DEBUG_LINE_H_INCLUDED

#include <WF3D/error.h>

#include <WF3D/Geometry/vect3d.h>
#include <WF3D/Geometry/quat.h>

#include <WF3D/Rendering/camera3d.h>
#include <WF3D/Rendering/Design/image_gen_interface.h>

#include <WF3D/Rendering/Design/color.h>

typedef struct
{
    wf3d_vect3d dir_vect;
    float t_max;
    float t_min;

    wf3d_color* color;

} wf3d_DebugLine;

//Create a DebugLine
wf3d_DebugLine* wf3d_DebugLine_Create(wf3d_vect3d dir_vect, float t_max, float t_min, wf3d_color* color);

//Destroy a debug line
void wf3d_DebugLine_Destroy(wf3d_DebugLine* line);

//
float wf3d_DebugLine_Radius(wf3d_DebugLine* line);

//
float wf3d_DebugLine_InfRadius(wf3d_DebugLine* line, wf3d_vect3d v_pos);

//
float wf3d_DebugLine_InfRadiusWithRot(wf3d_DebugLine* line, wf3d_vect3d v_pos, wf3d_quat q_rot);

//Rasterization function
//
wf3d_error wf3d_DebugLine_Rasterization(wf3d_DebugLine const* line, wf3d_img_gen_interface* img_out, float* depth_buffer, wf3d_vect3d v_pos, wf3d_quat q_rot, wf3d_camera3d const* cam);


#endif // WF3D_DEBUG_LINE_H_INCLUDED
