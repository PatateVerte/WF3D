#include <WF3D/Rendering/Object/cube.h>

#include <WF3D/Rendering/Shapes/triangle3d.h>

#include <math.h>

//Create a coloured cube
//surface_list[6]
//
wf3d_ColoredCube* wf3d_ColoredCube_Create(float side, wf3d_surface const* const* surface_list)
{
    wf3d_ColoredCube* cube = malloc(sizeof(*cube));

    if(cube != NULL)
    {
        cube->side = side;

        for(unsigned int k = 0 ; k < 6 ; k++)
        {
            cube->surface_list[k] = surface_list[k];
        }
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
float wf3d_ColoredCube_Radius(wf3d_ColoredCube const* cube)
{
    return 0.5 * OWL_SQRT3 * cube->side;
}

//
//
//
float wf3d_ColoredCube_InfRadius(wf3d_ColoredCube const* cube, owl_v3f32 v_pos)
{
    owl_v3f32 broadcast_half_side = owl_v3f32_broadcast(0.5 * cube->side);
    return fmaxf(
                    owl_v3f32_norminf(owl_v3f32_add(v_pos, broadcast_half_side)),
                    owl_v3f32_norminf(owl_v3f32_sub(v_pos, broadcast_half_side))
                 );
}

//
//
//
float wf3d_ColoredCube_InfRadiusWithRot(wf3d_ColoredCube const* cube, owl_v3f32 v_pos, owl_q32 q_rot)
{
    owl_v3f32 base_xyz[3];
    owl_v3f32_base_xyz(base_xyz, 0.5 * cube->side);

    for(unsigned int j = 0 ; j < 3 ; j++)
    {
        base_xyz[j] = owl_q32_transform_v3f32(q_rot, base_xyz[j]);
    }

    float inf_radius = 0.0;
    for(float sign_z = -1.0 ; sign_z <= 1.0 ; sign_z += 2.0)
    {
        owl_v3f32 center_z = owl_v3f32_add_scalar_mul(v_pos, base_xyz[2], sign_z);
        for(float sign_y = -1.0 ; sign_y <= 1.0 ; sign_y += 2.0)
        {
            owl_v3f32 center_y = owl_v3f32_add_scalar_mul(center_z, base_xyz[1], sign_y);
            for(float sign_x = -1.0 ; sign_x <= 1.0 ; sign_x += 2.0)
            {
                owl_v3f32 vertex = owl_v3f32_add_scalar_mul(center_y, base_xyz[0], sign_x);
                inf_radius = fmaxf(inf_radius, owl_v3f32_norminf(vertex));
            }
        }
    }

    return inf_radius;
}

//Rasterization function
//
//
wf3d_error wf3d_ColoredCube_Rasterization(wf3d_ColoredCube const* cube, wf3d_Image2d* img_out, wf3d_lightsource const* lightsource_list, unsigned int nb_lightsources, owl_v3f32 v_pos, owl_q32 q_rot, wf3d_camera3d const* cam)
{
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

        wf3d_triangle3d_Set(face_piece + 0, vertex_list + 0, base_xyz[bk], wf3d_triangle3d_MonoColorSurfaceCallback, cube->surface_list[bk]);
        wf3d_triangle3d_Set(face_piece + 1, vertex_list + 1, base_xyz[bk], wf3d_triangle3d_MonoColorSurfaceCallback, cube->surface_list[bk]);
        error = wf3d_triangle3d_Rasterization(face_piece + 0, img_out, lightsource_list, nb_lightsources, v_pos, q_rot, cam);
        if(error == WF3D_SUCCESS)
        {
            error = wf3d_triangle3d_Rasterization(face_piece + 1, img_out, lightsource_list, nb_lightsources, v_pos, q_rot, cam);

            if(error == WF3D_SUCCESS)
            {
                face_piece[0].normal = owl_v3f32_scalar_mul(base_xyz[bk], -1.0f);
                face_piece[0].design_data = cube->surface_list[3 + bk];
                face_piece[1].normal = owl_v3f32_scalar_mul(base_xyz[bk], -1.0f);
                face_piece[1].design_data = cube->surface_list[3 + bk];
                owl_v3f32 v_pos_behind = owl_v3f32_add_scalar_mul(
                                                                        v_pos,
                                                                        owl_q32_transform_v3f32(q_rot, adapted_base_xyz[bk]),
                                                                        -2.0
                                                                      );
                error = wf3d_triangle3d_Rasterization(face_piece + 0, img_out, lightsource_list, nb_lightsources, v_pos_behind, q_rot, cam);
                if(error == WF3D_SUCCESS)
                {
                    error = wf3d_triangle3d_Rasterization(face_piece + 1, img_out, lightsource_list, nb_lightsources, v_pos_behind, q_rot, cam);
                }
            }
        }
    }

    return error;
}
