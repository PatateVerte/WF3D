#include <WF3D/Rendering/Object/cube.h>

#include <WF3D/Rendering/Shapes/triangle3d.h>

#include <math.h>

#define M_SQRT3     1.73205080756887729352

//Create a coloured cube
//color_list[6]
//
wf3d_ColoredCube* wf3d_ColoredCube_Create(float side, wf3d_color const* color_list)
{
    wf3d_ColoredCube* cube = malloc(sizeof(*cube));

    if(cube != NULL)
    {
        cube->side = side;
        memcpy(cube->color_list, color_list, 6 * sizeof(*color_list));
    }

    return cube;
}

//
//
//
void wf3d_ColoredCube_Destroy(wf3d_ColoredCube* cube)
{
    if(cube != NULL)
    {
        free(cube);
    }
}

//
//
//
float wf3d_ColoredCube_Radius(wf3d_ColoredCube* cube)
{
    if(cube == NULL)
    {
        return 0.0;
    }

    return 0.5 * M_SQRT3 * cube->side;
}

//
//
//
float wf3d_ColoredCube_InfRadius(wf3d_ColoredCube* cube, owl_v3f32 v_pos)
{
    if(cube == NULL)
    {
        return 0.0;
    }

    owl_v3f32 base_xyz[3];
    owl_v3f32_base_xyz(base_xyz, 0.5 * cube->side);

    float inf_radius = 0.0;
    for(int k = 0 ; k < 3 ; k++)
    {
        float const tmp = fmaxf(
                                    owl_v3f32_norminf( owl_v3f32_add(v_pos, base_xyz[k]) ),
                                    owl_v3f32_norminf( owl_v3f32_sub(v_pos, base_xyz[k]) )
                                );
        inf_radius = fmaxf(inf_radius, tmp);
    }

    return inf_radius;
}

//
//
//
float wf3d_ColoredCube_InfRadiusWithRot(wf3d_ColoredCube* cube, owl_v3f32 v_pos, owl_q32 q_rot)
{
    if(cube == NULL)
    {
        return 0.0;
    }

    owl_v3f32 base_xyz[3];
    owl_v3f32_base_xyz(base_xyz, 0.5 * cube->side);

    float inf_radius = 0.0;
    for(int k = 0 ; k < 3 ; k++)
    {
        base_xyz[k] = owl_q32_transform_v3f32(q_rot, base_xyz[k]);
        float const tmp = fmaxf(
                                    owl_v3f32_norminf( owl_v3f32_add(v_pos, base_xyz[k]) ),
                                    owl_v3f32_norminf( owl_v3f32_sub(v_pos, base_xyz[k]) )
                                );
        inf_radius = fmaxf(inf_radius, tmp);
    }

    return inf_radius;
}

//Rasterization function
//
//
wf3d_error wf3d_ColoredCube_Rasterization(wf3d_ColoredCube const* cube, wf3d_Image3d* img_out, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam)
{
    if(cube == NULL)
    {
        return WF3D_SUCCESS;
    }

    wf3d_error error = WF3D_SUCCESS;

    owl_v3f32 base_xyz[3];
    owl_v3f32_base_xyz(base_xyz, 1.0f);

    owl_v3f32 adapted_base_xyz[3];
    owl_v3f32_base_xyz(adapted_base_xyz, 0.5f * cube->side);

    for(int bk = 0 ; bk < 3 && error == WF3D_SUCCESS ; bk++)
    {
        wf3d_triangle3d face_piece[2];

        owl_v3f32 const comple_base[2] =
        {
            adapted_base_xyz[(bk + 1) % 3],
            adapted_base_xyz[(bk + 2) % 3]
        };
        owl_v3f32 vertex_list[4] =
        {
            owl_v3f32_add( adapted_base_xyz[bk], owl_v3f32_add( comple_base[0], comple_base[1] ) ),
            owl_v3f32_add( adapted_base_xyz[bk], owl_v3f32_sub( comple_base[0], comple_base[1] ) ),
            owl_v3f32_sub( adapted_base_xyz[bk], owl_v3f32_sub( comple_base[0], comple_base[1] ) ),
            owl_v3f32_sub( adapted_base_xyz[bk], owl_v3f32_add( comple_base[0], comple_base[1] ) )
        };

        wf3d_triangle3d_Set(face_piece + 0, vertex_list + 0, base_xyz[bk], wf3d_triangle3d_MonoColorSurfaceCallback, cube->color_list + 2*bk);
        wf3d_triangle3d_Set(face_piece + 1, vertex_list + 1, base_xyz[bk], wf3d_triangle3d_MonoColorSurfaceCallback, cube->color_list + 2*bk);
        error = wf3d_triangle3d_Rasterization(face_piece + 0, img_out, v_pos, q_rot, cam);
        if(error == WF3D_SUCCESS)
        {
            error = wf3d_triangle3d_Rasterization(face_piece + 1, img_out, v_pos, q_rot, cam);

            if(error == WF3D_SUCCESS)
            {
                face_piece[0].normal = owl_v3f32_scalar_mul(base_xyz[bk], -1.0f);
                face_piece[0].design_data = cube->color_list + 2*bk + 1;
                face_piece[1].normal = owl_v3f32_scalar_mul(base_xyz[bk], -1.0f);
                face_piece[1].design_data = cube->color_list + 2*bk + 1;
                owl_v3f32 v_pos_behind = owl_v3f32_add_scalar_mul(
                                                                        v_pos,
                                                                        owl_q32_transform_v3f32(q_rot, adapted_base_xyz[bk]),
                                                                        -2.0
                                                                      );
                error = wf3d_triangle3d_Rasterization(face_piece + 0, img_out, v_pos_behind, q_rot, cam);
                if(error == WF3D_SUCCESS)
                {
                    error = wf3d_triangle3d_Rasterization(face_piece + 1, img_out, v_pos_behind, q_rot, cam);
                }
            }
        }
    }

    return error;
}
