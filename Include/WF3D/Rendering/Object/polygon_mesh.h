#ifndef WF3D_OBJECT3D_H_INCLUDED
#define WF3D_OBJECT3D_H_INCLUDED

#include <stdbool.h>

#include <OWL/v3f32.h>

#include <WF3D/error.h>
#include <WF3D/Rendering/camera3d.h>
#include <WF3D/Rendering/Shapes/triangle3d.h>
#include <WF3D/Rendering/Design/image3d.h>

typedef struct
{
    bool radius_has_changed;
    float radius;

    int nb_faces;
    wf3d_triangle3d* local_face_list;   //Coordinates are relative to the global positioning

} wf3d_PolygonMesh;

//
wf3d_PolygonMesh* wf3d_PolygonMesh_Create(int nb_faces);

//
void wf3d_PolygonMesh_Destroy(wf3d_PolygonMesh* obj);

//Copy the face to the face i
wf3d_triangle3d const* wf3d_PolygonMesh_ChangeFace(wf3d_PolygonMesh* obj, int i, wf3d_triangle3d const* new_face);

//
float wf3d_PolygonMesh_Radius(wf3d_PolygonMesh* obj);

//
float wf3d_PolygonMesh_InfRadius(wf3d_PolygonMesh* obj, owl_v3f32 v_pos);

//
float wf3d_PolygonMesh_InfRadiusWithRot(wf3d_PolygonMesh* obj, owl_v3f32 v_pos, owl_q32 q_rot);

//Rasterization function
//
wf3d_error wf3d_PolygonMesh_Rasterization(wf3d_PolygonMesh const* obj, wf3d_Image3d* img_out, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam);


#endif // WF3D_OBJECT3D_H_INCLUDED
