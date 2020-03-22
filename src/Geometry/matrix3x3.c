#include <WF3D/Geometry/matrix3x3.h>

#include <math.h>

//SSE
#include <xmmintrin.h>

//
//
//
wf3d_matrix3x3* wf3d_matrix3x3_diag(wf3d_matrix3x3* M, float diag_val)
{
    wf3d_vect3d_base_xyz(M->column, diag_val);

    return M;
}

//M = A + B
//
//
wf3d_matrix3x3* wf3d_matrix3x3_add(wf3d_matrix3x3* M, wf3d_matrix3x3 const* A, wf3d_matrix3x3 const* B)
{
    for(int j = 0 ; j < 3 ; j++)
    {
        M->column[j] = wf3d_vect3d_add(A->column[j], B->column[j]);
    }

    return M;
}

//M = A - B
//
//
wf3d_matrix3x3* wf3d_matrix3x3_sub(wf3d_matrix3x3* M, wf3d_matrix3x3 const* A, wf3d_matrix3x3 const* B)
{
    for(int j = 0 ; j < 3 ; j++)
    {
        M->column[j] = wf3d_vect3d_sub(A->column[j], B->column[j]);
    }

    return M;
}

//
//
//
wf3d_matrix3x3* wf3d_matrix3x3_scalar_mul(wf3d_matrix3x3* M, wf3d_matrix3x3 const* A, float a)
{
    for(int j = 0 ; j < 3 ; j++)
    {
        M->column[j] = wf3d_vect3d_scalar_mul(A->column[j], a);
    }

    return M;
}

//A * v
//
//
wf3d_vect3d wf3d_matrix3x3_transform(wf3d_matrix3x3 const* A, wf3d_vect3d v)
{
    float flat_vect[4] __attribute__( (aligned(16)) );
    wf3d_vect3d_store4(flat_vect, v);

    wf3d_vect3d vr = wf3d_vect3d_zero();

    for(int j = 0 ; j < 3 ; j++)
    {
        vr = wf3d_vect3d_add(
                                vr,
                                wf3d_vect3d_scalar_mul(A->column[j], flat_vect[j + 1])
                            );
    }

    return vr;
}

//M = A * B
//
//
wf3d_matrix3x3* wf3d_matrix3x3_mul(wf3d_matrix3x3* M, wf3d_matrix3x3 const* A, wf3d_matrix3x3 const* B)
{
    wf3d_matrix3x3 S;

    for(int j = 0 ; j < 3 ; j++)
    {
        S.column[j] = wf3d_matrix3x3_transform(A, B->column[j]);
    }

    wf3d_matrix3x3_copy(M, &S);

    return M;
}

//A = P * D * tP
//
//
wf3d_matrix3x3* wf3d_matrix3x3_diagonalize_sym(wf3d_matrix3x3* D, wf3d_matrix3x3* P, wf3d_matrix3x3 const* A)
{
    #define diag_nb_iter 25

    float vp_list[3] = {0.0, 0.0, 0.0};

    wf3d_matrix3x3 M;
    wf3d_matrix3x3_copy(&M, A);

    wf3d_matrix3x3 B;
    wf3d_matrix3x3_diag(&B, 1.0);

    wf3d_matrix3x3 VP_Base;
    wf3d_matrix3x3_zero(&VP_Base);

    for(int base_j = 0 ; base_j < 2 ; base_j++)
    {
        for(int k = 0 ; k < diag_nb_iter ; k++)
        {
            wf3d_matrix3x3_mul(&M, &M, &M);
            float norm2_M =  wf3d_matrix3x3_norm2(&M);
            wf3d_matrix3x3_scalar_mul(&M, &M, 1.0 / norm2_M);
        }

        wf3d_matrix3x3 T;
        wf3d_matrix3x3_mul(&T, &M, &B);

        int r = 0;
        float vp_abs = 0.0;
        for(int j = 0 ; j < 3 ; j++)
        {
            float tmp = wf3d_vect3d_norm( T.column[j] );
            if(tmp >= vp_abs)
            {
                r = j;
                vp_abs = tmp;
            }
        }

        VP_Base.column[base_j] = wf3d_vect3d_normalize( T.column[r] );
        vp_list[base_j] = wf3d_vect3d_dot(
                                            VP_Base.column[base_j],
                                            wf3d_matrix3x3_transform(A, VP_Base.column[base_j])
                                          );

        if(base_j == 0)
        {
            wf3d_matrix3x3_copy(&M, A);
            for(int j = 0 ; j < 3 ; j++)
            {
                M.column[j] = wf3d_vect3d_sub(
                                                M.column[j],
                                                wf3d_vect3d_scalar_mul(
                                                                        VP_Base.column[0],
                                                                        vp_list[0] * wf3d_vect3d_dot(VP_Base.column[0], B.column[j])
                                                                       )
                                               );
            }
        }
    }

    vp_list[2] = wf3d_matrix3x3_det(A) / (vp_list[0] * vp_list[1]);
    VP_Base.column[2] = wf3d_vect3d_cross(VP_Base.column[0], VP_Base.column[1]);

    for(int j = 0 ; j < 3 ; j++)
    {
        D->column[j] = wf3d_vect3d_scalar_mul(B.column[j], vp_list[j]);
    }

    if(P != NULL)
    {
        wf3d_matrix3x3_copy(P, &VP_Base);
    }

    return D;

    #undef diag_nb_iter
}
