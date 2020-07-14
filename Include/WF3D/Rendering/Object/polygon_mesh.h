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
    unsigned int nb_faces;
    wf3d_triangle3d* local_face_list;   //Coordinates are relative to the global positioning

    float radius;

} wf3d_PolygonMesh;

//Create a polygon mesh from a list of faces
wf3d_PolygonMesh* wf3d_PolygonMesh_Create(wf3d_triangle3d const* face_list, unsigned int nb_faces);

//
void wf3d_PolygonMesh_Destroy(wf3d_PolygonMesh* obj);

//Updates the internal stored radius of the mesh
wf3d_PolygonMesh* wf3d_PolygonMesh_UpdateRadius(wf3d_PolygonMesh* obj);

//Copy the face to the face i
wf3d_triangle3d const* wf3d_PolygonMesh_ChangeFace(wf3d_PolygonMesh* obj, unsigned int i, wf3d_triangle3d const* new_face);

//
float wf3d_PolygonMesh_Radius(wf3d_PolygonMesh const* obj);

//
float wf3d_PolygonMesh_InfRadius(wf3d_PolygonMesh const* obj, owl_v3f32 v_pos);

//
float wf3d_PolygonMesh_InfRadiusWithRot(wf3d_PolygonMesh const* obj, owl_v3f32 v_pos, owl_q32 q_rot);

//Rasterization function for quick ray tracing
wf3d_error wf3d_PolygonMesh_Rasterization(wf3d_PolygonMesh const* obj, wf3d_image2d_rectangle* img_out, wf3d_lightsource const* lightsource_list, unsigned int nb_lightsources, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam);


#endif // WF3D_OBJECT3D_H_INCLUDED
