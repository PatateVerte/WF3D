#include <WF3D/Geometry/quat.h>

#include <math.h>

//q1 * q2
//
//
wf3d_quat wf3d_quat_mul(wf3d_quat q1, wf3d_quat q2)
{
    //a
    __m128 qr = _mm_mul_ps(
                                _mm_shuffle_ps(q1, q1, 0b00000000),
                                _mm_shuffle_ps(q2, q2, 0b10110100)
                             );
    //c
    qr = _mm_addsub_ps(
                        qr,
                        _mm_mul_ps(
                                    _mm_shuffle_ps(q1, q1, 0b10101010),
                                    _mm_shuffle_ps(q2, q2, 0b00011110)
                                   )
                       );
    //d
    qr = _mm_addsub_ps(
                        _mm_shuffle_ps(qr, qr, 0b10011100),
                        _mm_mul_ps(
                                    _mm_shuffle_ps(q1, q1, 0b11111111),
                                    _mm_shuffle_ps(q2, q2, 0b00100111)
                                   )
                      );
    //b
    qr = _mm_addsub_ps(
                        _mm_shuffle_ps(qr, qr, 0b11011000),
                        _mm_mul_ps(
                                    _mm_shuffle_ps(q1, q1, 0b01010101),
                                    _mm_shuffle_ps(q2, q2, 0b10110001)
                                   )
                       );

    return qr;
}

//||v|| = 1
//Rotation of alpha around the unitary vector v
//
wf3d_quat wf3d_quat_rotation(wf3d_vect3d v, float alpha)
{
    float cos, sin;
    sincosf(0.5f * alpha, &sin, &cos);

    return _mm_insert_ps(
                            wf3d_vect3d_scalar_mul(v, sin),
                            _mm_set_ss(cos),
                            0b00000000
                        );
}

//|q| = 1
//(q) * u * (q^-1)
//
wf3d_vect3d wf3d_quat_transform_vect3d(wf3d_quat q, wf3d_vect3d u)
{
    float a = wf3d_quat_Ref(q);
    wf3d_vect3d v = wf3d_quat_Imv(q);

    wf3d_vect3d rot = wf3d_vect3d_scalar_mul(wf3d_vect3d_cross(v, u), a);
    rot = wf3d_vect3d_add_scalar_mul(
                                        rot,
                                        v,
                                        wf3d_vect3d_dot(v, u)
                                     );
    rot = wf3d_vect3d_add_scalar_mul(
                                        rot,
                                        u,
                                        a*a - 0.5f
                                     );

    return wf3d_vect3d_add(rot, rot);

}
