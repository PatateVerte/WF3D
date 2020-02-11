#ifndef WF3D_OBJECT3D_H_INCLUDED
#define WF3D_OBJECT3D_H_INCLUDED

#include <stdbool.h>

#include <WF3D/error.h>
#include <WF3D/Geometry/vect3d.h>
#include <WF3D/Rendering/camera3d.h>
#include <WF3D/Rendering/triangle3d.h>
#include <WF3D/Rendering/lightsource.h>

typedef struct
{
    wf3d_quat q_rot;
    wf3d_vect3d v_pos;

    int nb_faces;
    wf3d_triangle3d* local_face_list;   //Coordinates are relative to the global positioning

} wf3d_PolygonMesh;

//
wf3d_PolygonMesh* wf3d_PolygonMesh_Create(int nb_faces);

//
void wf3d_PolygonMesh_Destroy(wf3d_PolygonMesh* obj);

//Constant pointer to a face
wf3d_triangle3d const* wf3d_PolygonMesh_GetLocalFacePtr(wf3d_PolygonMesh const* obj, int i);

//Edit a face
wf3d_triangle3d const* wf3d_PolygonMesh_ChangeFace(wf3d_PolygonMesh* obj, int i, wf3d_triangle3d const* new_face);

//
static inline wf3d_PolygonMesh* wf3d_PolygonMesh_Move(wf3d_PolygonMesh* obj, wf3d_vect3d v)
{
    obj->v_pos = wf3d_vect3d_add(obj->v_pos, v);
    return obj;
}

//
wf3d_PolygonMesh* wf3d_PolygonMesh_Transform(wf3d_PolygonMesh* obj, wf3d_quat q_rot, wf3d_quat v);

//Rasterization function
//
wf3d_error wf3d_PolygonMesh_Rasterization(wf3d_PolygonMesh const* obj, wf3d_img_gen_interface* img_out, float* depth_buffer, wf3d_camera3d const* cam, wf3d_lightsource const* ls_list, int nb_ls);


#endif // WF3D_OBJECT3D_H_INCLUDED
