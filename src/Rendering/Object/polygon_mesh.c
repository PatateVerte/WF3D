#include <WF3D/Rendering/Object/polygon_mesh.h>

#include <math.h>
#include <malloc.h>

//
//
//
wf3d_PolygonMesh* wf3d_PolygonMesh_Create(wf3d_triangle3d const* face_list, unsigned int nb_faces)
{
    wf3d_PolygonMesh* obj = malloc(sizeof(*obj));

    if(obj != NULL)
    {
        obj->nb_faces = nb_faces;

        size_t face_buff_size = (size_t)nb_faces * sizeof(wf3d_triangle3d);
        obj->local_face_list = _aligned_malloc(face_buff_size, 16);

        if(nb_faces == 0 || obj->local_face_list != NULL)
        {
            for(unsigned int k = 0 ; k < nb_faces ; k++)
            {
                obj->local_face_list[k] = face_list[k];
            }

            wf3d_PolygonMesh_UpdateRadius(obj);
        }
        else
        {
            wf3d_PolygonMesh_Destroy(obj);
            obj = NULL;
        }
    }

    return obj;
}

//
//
//
void wf3d_PolygonMesh_Destroy(wf3d_PolygonMesh* obj)
{
    if(obj != NULL)
    {
        _aligned_free(obj->local_face_list);
        free(obj);
    }
}

//Updates the internal stored radius of the mesh
//
//
wf3d_PolygonMesh* wf3d_PolygonMesh_UpdateRadius(wf3d_PolygonMesh* obj)
{
    if(obj->nb_faces > 0)
    {
        float square_radius = 0.0;

        for(int fi = 0 ; fi < obj->nb_faces ; fi++)
        {
            for(int k = 0 ; k < 3 ; k++)
            {
                owl_v3f32 vertex = obj->local_face_list[fi].vertex_list[k];
                float tmp = owl_v3f32_dot(vertex, vertex);
                square_radius = fmaxf(square_radius, tmp);
            }
        }

        obj->radius = sqrtf(square_radius);
    }
    else
    {
        obj->radius = 0.0;
    }

    return obj;
}

//
//
//
wf3d_triangle3d const* wf3d_PolygonMesh_ChangeFace(wf3d_PolygonMesh* obj, unsigned int i, wf3d_triangle3d const* new_face)
{
    if(i < obj->nb_faces)
    {
        wf3d_triangle3d* local_face = obj->local_face_list + i;
        *local_face = *new_face;

        wf3d_PolygonMesh_UpdateRadius(obj);

        return local_face;
    }
    else
    {
        return NULL;
    }
}

//
//
//
float wf3d_PolygonMesh_Radius(wf3d_PolygonMesh const* obj)
{
    return obj->radius;
}

//
//
//
float wf3d_PolygonMesh_InfRadius(wf3d_PolygonMesh const* obj, owl_v3f32 v_pos)
{
    float inf_radius = 0.0;

    for(int fi = 0 ; fi < obj->nb_faces ; fi++)
    {
        for(int k = 0 ; k < 3 ; k++)
        {
            owl_v3f32 vertex = obj->local_face_list[fi].vertex_list[k];
            float tmp = owl_v3f32_norminf( owl_v3f32_add(vertex, v_pos) );
            inf_radius = fmaxf(inf_radius, tmp);
        }
    }

    return inf_radius;
}

//
//
//
float wf3d_PolygonMesh_InfRadiusWithRot(wf3d_PolygonMesh const* obj, owl_v3f32 v_pos, owl_q32 q_rot)
{
    float inf_radius = 0.0;

    for(int fi = 0 ; fi < obj->nb_faces ; fi++)
    {
        for(int k = 0 ; k < 3 ; k++)
        {
            owl_v3f32 vertex = obj->local_face_list[fi].vertex_list[k];
            float tmp = owl_v3f32_norminf( owl_v3f32_add( owl_q32_transform_v3f32(q_rot, vertex), v_pos) );
            inf_radius = fmaxf(inf_radius, tmp);
        }
    }

    return inf_radius;
}

//
//
//
wf3d_error wf3d_PolygonMesh_Rasterization(wf3d_PolygonMesh const* obj, wf3d_Image2d* img_out, wf3d_lightsource const* lightsource_list, unsigned int nb_lightsources, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam)
{
    if(obj == NULL)
    {
        return WF3D_SUCCESS;
    }

    wf3d_error error = WF3D_SUCCESS;

    for(int fi = 0 ; fi < obj->nb_faces && error == WF3D_SUCCESS ; fi++)
    {
        error = wf3d_triangle3d_Rasterization(obj->local_face_list + fi, img_out, lightsource_list, nb_lightsources, v_pos, q_rot, cam);
    }

    return error;
}
