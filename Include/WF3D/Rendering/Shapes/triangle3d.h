#ifndef WF3D_TRIANGLE_H_INCLUDED
#define WF3D_TRIANGLE_H_INCLUDED

#include <WF3D/Geometry/vect3d.h>
#include <WF3D/Geometry/matrix3x3.h>
#include <WF3D/Geometry/quat.h>

#include <WF3D/error.h>

#include <WF3D/Rendering/camera3d.h>
#include <WF3D/Rendering/Design/image_gen_interface.h>

#include <stdbool.h>

typedef struct
{
    wf3d_vect3d vertex_list[3];
    wf3d_vect3d normal;

    //wf3d_color* color_of(void* design_data_ptr, wf3d_color* color_ret, float const* barycentric_coords)
    wf3d_color* (*color_of)(void const*, wf3d_color*, float const*);
    void const* design_data;

} wf3d_triangle3d;

wf3d_triangle3d* wf3d_triangle3d_Set(
                                        wf3d_triangle3d* triangle, wf3d_vect3d* vertex_list, wf3d_vect3d normal,
                                        wf3d_color* (*color_of)(void const*, wf3d_color*, float const*),
                                        void const* design_data
                                     );

static inline wf3d_triangle3d* wf3d_triangle3d_CopyDesign(wf3d_triangle3d* t_dst, wf3d_triangle3d const* t_src)
{
    t_dst->color_of = t_src->color_of;
    t_dst->design_data = t_src->design_data;

    return t_dst;
}

//
wf3d_triangle3d* wf3d_triangle3d_ComputeNormal(wf3d_triangle3d* triangle);

//Move triangle
wf3d_triangle3d* wf3d_triangle3d_Move(wf3d_triangle3d* t_dst, wf3d_triangle3d const* t_src, wf3d_vect3d v);

//Transform triangle (only the coordinates)
wf3d_triangle3d* wf3d_triangle3d_GeometricTransform(wf3d_triangle3d* t_dst, wf3d_triangle3d const* t_src, wf3d_quat q_rot, wf3d_vect3d v);

//Transform triangle and copy data
wf3d_triangle3d* wf3d_triangle3d_CopyAndTransform(wf3d_triangle3d* t_dst, wf3d_triangle3d const* t_src, wf3d_quat q_rot, wf3d_vect3d v);

//Rasterization of a triangle
wf3d_error wf3d_triangle3d_Rasterization(wf3d_triangle3d const* triangle, wf3d_img_gen_interface* img_out, float* depth_buffer, wf3d_vect3d v_pos, wf3d_quat q_rot, wf3d_camera3d const* cam);

typedef struct
{
    wf3d_color vertex_color_list[3];

} wf3d_triangle3d_tricolor_design;

typedef struct
{
    //vertices_barycentric_coords[vertex_i][barycentric_coord_i]
    float vertices_barycentric_coords[3][3];

    wf3d_color* (*original_color_of)(void const*, wf3d_color*, float const*);
    void const* original_design;

} wf3d_triangle3d_clipped_design;

//design_data_ptr : wf3d_color*
wf3d_color* wf3d_triangle3d_MonoColorSurfaceCallback(void const* design_data_ptr, wf3d_color* color_ret, float const* barycentric_coords);

//design_data_ptr : wf3d_triangle3d_tricolor_design*
wf3d_color* wf3d_triangle3d_TriColorSurfaceCallback(void const* design_data_ptr, wf3d_color* color_ret, float const* barycentric_coords);

//design_data_ptr : wf3d_triangle3d_clipped_design*
wf3d_color* wf3d_triangle3d_ClippedDesignCallback(void const* design_data_ptr, wf3d_color* color_ret, float const* barycentric_coords);

#endif // WF3D_TRIANGLE_H_INCLUDED
