#ifndef WF3D_CAMERA3D_H_INCLUDED
#define WF3D_CAMERA3D_H_INCLUDED

#include <stdbool.h>

#include <WF3D/Geometry/vect3d.h>
#include <WF3D/Geometry/quat.h>
#include <WF3D/Geometry/matrix3x3.h>

typedef struct
{
    wf3d_quat q_rot; //|q_rot| = 1
    wf3d_vect3d v_pos;

    float render_distance;
    bool blackface_culling_enabled;

    float tan_h_half_opening_angle;
    float tan_v_half_opening_angle;

} wf3d_camera3d;

//
wf3d_camera3d* wf3d_camera3d_Set(wf3d_camera3d* cam, float render_distance, float h_opening_angle, float v_opening_angle, bool blackface_culling_enabled);

#endif // WF3D_CAMERA3D_H_INCLUDED
