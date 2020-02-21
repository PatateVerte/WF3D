#include <WF3D/Rendering/Object/polygon_mesh.h>

#include <math.h>
#include <malloc.h>

//
//
//
wf3d_PolygonMesh* wf3d_PolygonMesh_Create(int nb_faces)
{
    wf3d_PolygonMesh* obj = NULL;
    bool error = false;

    obj = malloc(sizeof(*obj));
    if(obj != NULL)
    {
        obj->nb_faces = nb_faces;
        obj->local_face_list = NULL;

		obj->radius = 0.0;
		obj->radius_has_changed = true;

        if(nb_faces >= 0)
        {
            size_t face_buff_size = (size_t)nb_faces * sizeof(wf3d_triangle3d);
            obj->local_face_list = _aligned_malloc(face_buff_size, 16);

            if(obj->local_face_list == NULL)
            {
                error = true;
            }
        }
        else
        {
            error = true;
        }
    }
    else
    {
        error = true;
    }

    if(error)
    {
        wf3d_PolygonMesh_Destroy(obj);
        obj = NULL;
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

//
//
//
wf3d_triangle3d const* wf3d_PolygonMesh_ChangeFace(wf3d_PolygonMesh* obj, int i, wf3d_triangle3d const* new_face)
{
    if(obj == NULL)
    {
        return NULL;
    }

    if(i >= 0 && i < obj->nb_faces)
    {
        wf3d_triangle3d* local_face = obj->local_face_list + i;
        *local_face = *new_face;

        obj->radius_has_changed = true;

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
float wf3d_PolygonMesh_Radius(wf3d_PolygonMesh* obj)
{
    if(obj == NULL)
    {
        return 0.0;
    }

    if(obj->radius_has_changed)
    {
        float square_radius = 0.0;

        for(int fi = 0 ; fi < obj->nb_faces ; fi++)
        {
            for(int k = 0 ; k < 3 ; k++)
            {
                wf3d_vect3d vertex = obj->local_face_list[fi].vertex_list[k];
                float tmp = wf3d_vect3d_dot(vertex, vertex);
                square_radius = fmaxf(square_radius, tmp);
            }
        }

        float radius = sqrtf(square_radius);
        obj->radius = radius;
        obj->radius_has_changed = false;
        return radius;
    }
    else
    {
        return obj->radius;
    }
}

//
//
//
float wf3d_PolygonMesh_InfRadius(wf3d_PolygonMesh* obj, wf3d_vect3d v_pos)
{
    if(obj == NULL)
    {
        return 0.0;
    }

    float inf_radius = 0.0;

    for(int fi = 0 ; fi < obj->nb_faces ; fi++)
    {
        for(int k = 0 ; k < 3 ; k++)
        {
            wf3d_vect3d vertex = obj->local_face_list[fi].vertex_list[k];
            float tmp = wf3d_vect3d_inf_norm( wf3d_vect3d_add(vertex, v_pos) );
            inf_radius = fmaxf(inf_radius, tmp);
        }
    }

    return inf_radius;
}

//
//
//
float wf3d_PolygonMesh_InfRadiusWithRot(wf3d_PolygonMesh* obj, wf3d_vect3d v_pos, wf3d_quat q_rot)
{
    if(obj == NULL)
    {
        return 0.0;
    }

    float inf_radius = 0.0;

    for(int fi = 0 ; fi < obj->nb_faces ; fi++)
    {
        for(int k = 0 ; k < 3 ; k++)
        {
            wf3d_vect3d vertex = obj->local_face_list[fi].vertex_list[k];
            float tmp = wf3d_vect3d_inf_norm( wf3d_vect3d_add( wf3d_quat_transform_vect3d(q_rot, vertex), v_pos) );
            inf_radius = fmaxf(inf_radius, tmp);
        }
    }

    return inf_radius;
}

//
//
//
wf3d_error wf3d_PolygonMesh_Rasterization(wf3d_PolygonMesh const* obj, wf3d_Image3d* img_out, wf3d_vect3d v_pos, wf3d_quat q_rot, wf3d_camera3d const* cam)
{
    if(obj == NULL)
    {
        return WF3D_SUCCESS;
    }

    wf3d_error error = WF3D_SUCCESS;

    for(int fi = 0 ; fi < obj->nb_faces && error == WF3D_SUCCESS ; fi++)
    {
        error = wf3d_triangle3d_Rasterization(obj->local_face_list + fi, img_out, v_pos, q_rot, cam);
    }

    return error;
}
