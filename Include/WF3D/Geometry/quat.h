#ifndef WF3D_QUAT_H_INCLUDED
#define WF3D_QUAT_H_INCLUDED

#include <xmmintrin.h>
#include <pmmintrin.h>
#include <smmintrin.h>

#include <WF3D/Geometry/vect3d.h>

typedef __m128 wf3d_quat;

static inline wf3d_quat wf3d_quat_zero()
{
    return _mm_setzero_ps();
}

//Sets a quaternion (w = Real part, (x, y, z) = vect part)
static inline wf3d_quat wf3d_quat_set(float w, float x, float y, float z)
{
    return _mm_set_ps(z, y, x, w);
}

//Load quaternion
static inline wf3d_quat wf3d_quat_load4(float const* src)
{
    return _mm_load_ps(src);
}

//Store quaternion
static inline float* wf3d_quat_store4(float* dst, wf3d_quat q)
{
    _mm_store_ps(dst, q);
    return dst;
}

//Re(q) as a float
static inline float wf3d_quat_Ref(wf3d_quat q)
{
    return _mm_cvtss_f32(q);
}

//Im(q) as a vector
static inline wf3d_vect3d wf3d_quat_Imv(wf3d_quat q)
{
    return _mm_insert_ps(q, q, 0b00000001);
}

//Re(q) as a quat
static inline wf3d_quat wf3d_quat_Re(wf3d_quat q)
{
    return _mm_insert_ps(q, q, 0b00001110);
}

//Im(q)
static inline wf3d_quat wf3d_quat_Im(wf3d_quat q)
{
    return _mm_insert_ps(q, q, 0b00000001);
}

//|q1|^2
static inline float wf3d_quat_square_mod(wf3d_quat q)
{
    return _mm_cvtss_f32( _mm_dp_ps(q, q, 0b11110001) );
}

//|q1|
static inline float wf3d_quat_mod(wf3d_quat q)
{
    return _mm_cvtss_f32( _mm_sqrt_ss( _mm_dp_ps(q, q, 0b11110001) ) );
}

//q1 + q2
static inline wf3d_quat wf3d_quat_add(wf3d_quat q1, wf3d_quat q2)
{
    return _mm_add_ps(q1, q2);
}

//q1 - q2
static inline wf3d_quat wf3d_quat_sub(wf3d_quat q1, wf3d_quat q2)
{
    return _mm_sub_ps(q1, q2);
}

//Conj(q)
static inline wf3d_quat wf3d_quat_conj(wf3d_quat q)
{
    return wf3d_quat_sub(
                            wf3d_quat_Re(q),
                            wf3d_quat_Im(q)
                         );
}

//a * q
static inline wf3d_quat wf3d_quat_real_mul(wf3d_quat q, float a)
{
    return _mm_mul_ps(q, _mm_set1_ps(a));
}

//q / a
static inline wf3d_quat wf3d_quat_real_div(wf3d_quat q, float a)
{
    return _mm_div_ps(q, _mm_set1_ps(a));
}

//Inv(q)
static inline wf3d_quat wf3d_quat_inv(wf3d_quat q)
{
    return wf3d_quat_real_div(
                                wf3d_quat_conj(q),
                                wf3d_quat_square_mod(q)
                              );
}

//q1 * q2
wf3d_quat wf3d_quat_mul(wf3d_quat q1, wf3d_quat q2);

//||v|| = 1
//Rotation of alpha around the unitary vector v
wf3d_quat wf3d_quat_rotation(wf3d_vect3d v, float alpha);

//|q| = 1
//(q) * u * (q^-1)
wf3d_vect3d wf3d_quat_transform_vect3d(wf3d_quat q, wf3d_vect3d u);

#endif // WF3D_QUAT_H_INCLUDED
