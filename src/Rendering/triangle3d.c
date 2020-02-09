#include <WF3D/Rendering/triangle3d.h>

//
//
//
wf3d_triangle3d* wf3d_triangle3d_ComputeNormal(wf3d_triangle3d* triangle)
{
    wf3d_vect3d v1 = triangle->vertex_list[1] - triangle->vertex_list[0];
    wf3d_vect3d v2 = triangle->vertex_list[2] - triangle->vertex_list[0];

    triangle->normal = wf3d_vect3d_normalize(wf3d_vect3d_cross(v1, v2));

    return triangle;
}

//
//
//
wf3d_triangle3d* wf3d_triangle3d_Move(wf3d_triangle3d* t_dst, wf3d_triangle3d const* t_src, wf3d_vect3d v)
{
    for(int i = 0 ; i < 3 ; i++)
    {
        t_dst->vertex_list[i] = wf3d_vect3d_add(t_dst->vertex_list[i], v);
    }

    return wf3d_triangle3d_CopyDesign(t_dst, t_src);
}

//
//
//
wf3d_triangle3d* wf3d_triangle3d_GeometricTransform(wf3d_triangle3d* t_dst, wf3d_triangle3d const* t_src, wf3d_quat q_rot, wf3d_vect3d v)
{
    for(int i = 0 ; i < 3 ; i++)
    {
        t_dst->vertex_list[i] = wf3d_quat_add(
                                                wf3d_quat_transform_vect3d(q_rot, t_src->vertex_list[i]),
                                                v
                                             );
    }
    t_dst->normal = wf3d_quat_transform_vect3d(q_rot, t_src->normal);

    return t_dst;
}

//
//
//
wf3d_triangle3d* wf3d_triangle3d_CopyAndTransform(wf3d_triangle3d* t_dst, wf3d_triangle3d const* t_src, wf3d_quat q_rot, wf3d_vect3d v)
{
    for(int i = 0 ; i < 3 ; i++)
    {
        t_dst->vertex_list[i] = wf3d_quat_add(
                                                wf3d_quat_transform_vect3d(q_rot, t_src->vertex_list[i]),
                                                v
                                             );
    }
    t_dst->normal = wf3d_quat_transform_vect3d(q_rot, t_src->normal);

    return wf3d_triangle3d_CopyDesign(t_dst, t_src);
}

//
//
//
wf3d_color* wf3d_triangle3d_MonoColorSurfaceCallback(void* design_data_ptr, wf3d_color* color_ret, float const* barycentric_coords)
{
    wf3d_color* design_data = design_data_ptr;
    *color_ret = *design_data;

    return color_ret;
}

//
//
//
wf3d_color* wf3d_triangle3d_TriColorSurfaceCallback(void* design_data_ptr, wf3d_color* color_ret, float const* barycentric_coords)
{
    wf3d_triangle3d_tricolor_design const* design_data = design_data_ptr;

    wf3d_color_mix_colors(color_ret, design_data->vertex_color_list, barycentric_coords, 3);

    return color_ret;
}
