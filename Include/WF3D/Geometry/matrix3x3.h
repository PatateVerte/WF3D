#ifndef WF3D_MATRIX3X3_H_INCLUDED
#define WF3D_MATRIX3X3_H_INCLUDED

#include <WF3D/Geometry/vect3d.h>

//SSE / 128-bits AVX
#include <xmmintrin.h>
#include <pmmintrin.h>
#include <smmintrin.h>

#include <string.h>

typedef struct
{
    wf3d_vect3d column[3];

} wf3d_matrix3x3;

//M = 0
//Return M
static inline wf3d_matrix3x3* wf3d_matrix3x3_zero(wf3d_matrix3x3* M)
{
	for(int j = 0 ; j < 3 ; j++)
    {
        M->column[j] = wf3d_vect3d_zero();
    }

    return M;
}

//
wf3d_matrix3x3* wf3d_matrix3x3_diag(wf3d_matrix3x3* M, float diag_val);

//
wf3d_matrix3x3* wf3d_matrix3x3_set_element(wf3d_matrix3x3* M, float value, int i, int j);

//M = A + B
wf3d_matrix3x3* wf3d_matrix3x3_add(wf3d_matrix3x3* M, wf3d_matrix3x3 const* A, wf3d_matrix3x3 const* B);

//M = A - B
wf3d_matrix3x3* wf3d_matrix3x3_sub(wf3d_matrix3x3* M, wf3d_matrix3x3 const* A, wf3d_matrix3x3 const* B);

//M = a * A
wf3d_matrix3x3* wf3d_matrix3x3_scalar_mul(wf3d_matrix3x3* M, wf3d_matrix3x3 const* A, float a);

//Transform vector with A
wf3d_vect3d wf3d_matrix3x3_transform(wf3d_matrix3x3 const* A, wf3d_vect3d v);


//M = A * B
wf3d_matrix3x3* wf3d_matrix3x3_mul(wf3d_matrix3x3* M, wf3d_matrix3x3 const* A, wf3d_matrix3x3 const* B);

//M = A
static inline wf3d_matrix3x3* wf3d_matrix3x3_copy(wf3d_matrix3x3* M, wf3d_matrix3x3 const* A)
{
    for(int j = 0 ; j < 3 ; j++)
    {
        M->column[j] = A->column[j];
    }
    return M;
}

//det(A)
static inline float wf3d_matrix3x3_set(wf3d_matrix3x3 const* A)
{
    return wf3d_vect3d_triple(A->column[0], A->column[1], A->column[2]);
}

#endif // WF3D_MATRIX3X3_H_INCLUDED
