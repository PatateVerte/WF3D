#include <WF3D/Geometry/matrix3x3.h>

#include <math.h>

//SSE
#include <xmmintrin.h>

//
//
//
wf3d_matrix3x3* wf3d_matrix3x3_diag(wf3d_matrix3x3* M, float diag_val)
{
    wf3d_vect3d base = wf3d_vect3d_set(diag_val, 0.0, 0.0);
    for(int j = 0 ; j < 3 ; j++)
    {
        M->column[j] = base;
        base = wf3d_vect3d_rotate_comp(base);
    }

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
        wf3d_vect3d_add_scalar_mul(vr, A->column[j], flat_vect[j + 1]);
    }

    return vr;
}

//M = A * B
//
//
wf3d_matrix3x3* wf3d_matrix3x3_mul(wf3d_matrix3x3* M, wf3d_matrix3x3 const* A, wf3d_matrix3x3 const* B)
{
    wf3d_matrix3x3 T;

    wf3d_matrix3x3* S = (M != A) ? M : &T;

    for(int j = 0 ; j < 3 ; j++)
    {
        S->column[j] = wf3d_matrix3x3_transform(A, B->column[j]);
    }

    if(M != S)
    {
        wf3d_matrix3x3_copy(M, S);
    }

    return M;
}
