#ifndef WF3D_VECT3D_H_INCLUDED
#define WF3D_VECT3D_H_INCLUDED

#include <math.h>

//SSE / 128-bits AVX
#include <xmmintrin.h>
#include <pmmintrin.h>
#include <smmintrin.h>

typedef __m128 wf3d_vect3d;

//null vector
static inline wf3d_vect3d wf3d_vect3d_zero()
{
    return _mm_setzero_ps();
}

//(x, y, z) vector
static inline wf3d_vect3d wf3d_vect3d_set(float x, float y, float z)
{
    return _mm_set_ps(z, y, x, 0.0);
}

//(w, w, w) vector
static inline wf3d_vect3d wf3d_vect3d_set1(float w)
{
    return _mm_set1_ps(w);
}

//Vector from a float[4]
static inline wf3d_vect3d wf3d_vect3d_load4(const float* src)
{
    return _mm_load_ps(src);
}

//Vector to float[4]
static inline float* wf3d_vect3d_store4(float* dst, wf3d_vect3d v)
{
    _mm_store_ps(dst, v);
    return dst;
}

//a + b
static inline wf3d_vect3d wf3d_vect3d_add(wf3d_vect3d a, wf3d_vect3d b)
{
    return _mm_add_ps(a, b);
}

//a - b
static inline wf3d_vect3d wf3d_vect3d_sub(wf3d_vect3d a, wf3d_vect3d b)
{
    return _mm_sub_ps(a, b);
}

//
static inline wf3d_vect3d wf3d_vect3d_unsafe_set_component(wf3d_vect3d v, int i, float val)
{
    float tmp[4] __attribute__((aligned(16)));
    _mm_store_ps(tmp, v);
    tmp[i + 1] = val;
    return _mm_load_ps(tmp);
}

//
static inline float wf3d_vect3d_unsafe_get_component(wf3d_vect3d v, int i)
{
    float tmp[4] __attribute__((aligned(16))) ;
    _mm_store_ps(tmp, v);
    return tmp[i + 1];
}

//
static inline float wf3d_vect3d_get_component(wf3d_vect3d v, int i)
{
    float tmp[4] __attribute__((aligned(16))) ;
    _mm_store_ps(tmp, v);

    if(i < 4 && i >= 0)
    {
        return tmp[i + 1];
    }
    else
    {
        return NAN;
    }
}

//a * v
static inline wf3d_vect3d wf3d_vect3d_scalar_mul(wf3d_vect3d v, float a)
{
    return _mm_mul_ps(v, _mm_set1_ps(a));
}

//v / a
static inline wf3d_vect3d wf3d_vect3d_scalar_div(wf3d_vect3d v, float a)
{
    return _mm_div_ps(v, _mm_set1_ps(a));
}

//v1 + a*v2
static inline wf3d_vect3d wf3d_vect3d_add_scalar_mul(wf3d_vect3d v1, wf3d_vect3d v2, float a)
{
    return _mm_add_ps(
                        v1,
                        _mm_mul_ps( _mm_set1_ps(a), v2 )
                      );
}

//a.b
static inline float wf3d_vect3d_dot(wf3d_vect3d a, wf3d_vect3d b)
{
    return _mm_cvtss_f32( _mm_dp_ps(a, b, 0b11100001) );
}

//||a||
static inline float wf3d_vect3d_norm(wf3d_vect3d a)
{
    return _mm_cvtss_f32( _mm_sqrt_ss( _mm_dp_ps(a, a, 0b11100001) ) );
}

//1 / ||a||
static inline float wf3d_vect3d_inv_norm(wf3d_vect3d a)
{
    __m128 square_dot = _mm_dp_ps(a, a, 0b11100001);
    const float square_norm = _mm_cvtss_f32(square_dot);
    float inv_norm = _mm_cvtss_f32( _mm_rsqrt_ss( square_dot ) );

    //Newton additional iteration
    inv_norm = 0.5f * (3.0f - square_norm * inv_norm * inv_norm) * inv_norm;

    return inv_norm;
}

//
static inline wf3d_vect3d wf3d_vect3d_normalize(wf3d_vect3d a)
{
    return wf3d_vect3d_scalar_mul(a, wf3d_vect3d_inv_norm(a) );
}

//a^b
static inline wf3d_vect3d wf3d_vect3d_cross(wf3d_vect3d a, wf3d_vect3d b)
{
    __m128 tmp = _mm_mul_ps(
                                _mm_shuffle_ps(a, a, 0b01111000),
                                _mm_shuffle_ps(b, b, 0b10011100)
                            );

    tmp = _mm_sub_ps(
                     tmp,
                     _mm_mul_ps(
                                    _mm_shuffle_ps(a, a, 0b10011100),
                                    _mm_shuffle_ps(b, b, 0b01111000)
                                )
                    );

    return tmp;
}

//[a, b, c]
static inline float wf3d_vect3d_triple(wf3d_vect3d a, wf3d_vect3d b, wf3d_vect3d c)
{
    return wf3d_vect3d_dot(wf3d_vect3d_cross(a, b), c);
}

//
static inline wf3d_vect3d wf3d_vect3d_rotate_comp(wf3d_vect3d v)
{
    return _mm_shuffle_ps(v, v, 0b10011100);
}

#endif // WF3D_VECT3D_H_INCLUDED
