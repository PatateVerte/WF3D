#ifndef WF3D_OBJECT3D_H_INCLUDED
#define WF3D_OBJECT3D_H_INCLUDED

#include <stdbool.h>

#include <WF3D/error.h>
#include <WF3D/Geometry/vect3d.h>
#include <WF3D/Rendering/triangle3d.h>

typedef struct
{
    wf3d_quat q_rot;
    wf3d_vect3d v_pos;

    int nb_faces;
    wf3d_triangle3d* local_face_list;   //Coordinates are relative to the global positioning

} wf3d_Object3d;

//
wf3d_Object3d* wf3d_Object3d_Create(int nb_faces);

//
void wf3d_Object3d_Destroy(wf3d_Object3d* obj);

//Constant pointer to a face
wf3d_triangle3d const* wf3d_Object3d_GetLocalFacePtr(wf3d_Object3d const* obj, int i);

//Edit a face
wf3d_triangle3d const* wf3d_Object3d_ChangeFace(wf3d_Object3d* obj, int i, wf3d_triangle3d const* new_face);

//
static inline wf3d_Object3d* wf3d_Object3d_Move(wf3d_Object3d* obj, wf3d_vect3d v)
{
    obj->v_pos = wf3d_vect3d_add(obj->v_pos, v);
    return obj;
}

//
wf3d_Object3d* wf3d_Object3d_Transform(wf3d_Object3d* obj, wf3d_quat q_rot, wf3d_quat v);

//Rasterization function
//
wf3d_error wf3d_Object3d_Rasterization(wf3d_Object3d const* obj, wf3d_img_gen_interface* img_out, float* depth_buffer, wf3d_vect3d cam_pos, wf3d_quat cam_rot, float tan_h_half_opening_angle, float tan_v_half_opening_angle, float render_distance, bool blackface_culling_enabled);


#endif // WF3D_OBJECT3D_H_INCLUDED
