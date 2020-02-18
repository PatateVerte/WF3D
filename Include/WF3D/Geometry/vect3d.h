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

//Fill base[3] with the basic trihedral multiplied by f
static inline wf3d_vect3d* wf3d_vect3d_base_xyz(wf3d_vect3d* base, float f)
{
    __m128 tmp = _mm_set_ss(f);
    base[0] = _mm_insert_ps(tmp, tmp, 0b00010001);
    base[1] = _mm_insert_ps(tmp, tmp, 0b00100001);
    base[2] = _mm_insert_ps(tmp, tmp, 0b00110001);

    return base;
}

//Broadcast f within a vector
static inline wf3d_vect3d wf3d_vect3d_broadcast(float f)
{
    __m128 tmp = _mm_set1_ps(f);
    return _mm_insert_ps(tmp, tmp, 0b00000001);
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
#define wf3d_vect3d_set_component(v, i, val) \
    _mm_insert_ps( (v) , _mm_set_ss((val)) , (0b0001 | (0b00 << 6) | ((i + 1) << 4)) )


#define wf3d_vect3d_get_component(v, i) \
    _mm_cvtss_f32( _mm_insert_ps((v) , (v) , (0b1110 | ((i + 1) << 6) | (0b00 << 4)) ) )

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

//Infinity norm
static inline float wf3d_vect3d_inf_norm(wf3d_vect3d v)
{
    __m128 v_abs = _mm_max_ps(
                                v,
                                _mm_sub_ps(_mm_setzero_ps(), v)
                              );

    __m128 tmp = _mm_insert_ps(v_abs, v_abs, 0b01001110);
    tmp = _mm_max_ss(tmp, _mm_insert_ps(v_abs, v_abs, 0b10001110));
    tmp = _mm_max_ss(tmp, _mm_insert_ps(v_abs, v_abs, 0b11001110));

    return _mm_cvtss_f32(tmp);
}

//Sign mask in int[2:0]
//0 : < 0
//1 : >= 0
static inline int wf3d_vect3d_sign_mask(wf3d_vect3d v)
{
    __m128i tmp = _mm_castps_si128( _mm_cmpge_ps(v, _mm_setzero_ps()) );
    tmp = _mm_packs_epi32(tmp, tmp);
    tmp = _mm_packs_epi16(tmp, tmp);

    int tmp_mask = _mm_cvtsi128_si32(tmp);
    tmp_mask &= (1<<8) | (1<<16) | (1<<24);
    int sign_mask = (tmp_mask>>8) | (tmp_mask >> 15) | (tmp_mask >> 22);
    sign_mask &= 0b111;

    return sign_mask;
}

#endif // WF3D_VECT3D_H_INCLUDED
