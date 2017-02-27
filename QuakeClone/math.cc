#include "shared.h"

void Mat1x3Mul(Vec3 *out, const Vec3 *a, const r32 b[3][3]) {
	r32 a00 = (*a)[0], a01 = (*a)[1], a02 = (*a)[2];

	r32 b00 = b[0][0], b01 = b[0][1], b02 = b[0][2];
	r32 b10 = b[1][0], b11 = b[1][1], b12 = b[1][2];
	r32 b20 = b[2][0], b21 = b[2][1], b22 = b[2][2];

	(*out)[0] = a00*b00 + a01*b10 + a02*b20;
	(*out)[1] = a00*b01 + a01*b11 + a02*b21;
	(*out)[2] = a00*b02 + a01*b12 + a02*b22;
}

void Mat1x4Mul(r32 out[4], const r32 a[4], const r32 b[4][4]) {
	r32 a00 = a[0], a01 = a[1], a02 = a[2], a03 = a[3];

	r32 b00 = b[0][0], b01 = b[0][1], b02 = b[0][2], b03 = b[0][3];
	r32 b10 = b[1][0], b11 = b[1][1], b12 = b[1][2], b13 = b[1][3];
	r32 b20 = b[2][0], b21 = b[2][1], b22 = b[2][2], b23 = b[2][3];
	r32 b30 = b[3][0], b31 = b[3][1], b32 = b[3][2], b33 = b[3][3];

	out[0] = a00*b00 + a01*b10 + a02*b20 + a03*b30;
	out[1] = a00*b01 + a01*b11 + a02*b21 + a03*b31;
	out[2] = a00*b02 + a01*b12 + a02*b22 + a03*b32;
	out[3] = a00*b03 + a01*b13 + a02*b23 + a03*b33;
}

void Mat2x2Mul(r32 out[2][2], const r32 a[2][2], const r32 b[2][2]) {
	r32 a00 = a[0][0], a01 = a[0][1];
	r32 a10 = a[1][0], a11 = a[1][1];

	r32 b00 = b[0][0], b01 = b[0][1];
	r32 b10 = b[1][0], b11 = b[1][1];

	out[0][0] = a00*b00 + a01*b10;
	out[0][1] = a00*b01 + a01*b11;

	out[1][0] = a10*b00 + a11*b10;
	out[1][1] = a10*b01 + a11*b11;
}

void Mat3x3Mul(r32 out[3][3], const r32 a[3][3], const r32 b[3][3]) {
	r32 a00 = a[0][0], a01 = a[0][1], a02 = a[0][2];
	r32 a10 = a[1][0], a11 = a[1][1], a12 = a[1][2];
	r32 a20 = a[2][0], a21 = a[2][1], a22 = a[2][2];

	r32 b00 = b[0][0], b01 = b[0][1], b02 = b[0][2];
	r32 b10 = b[1][0], b11 = b[1][1], b12 = b[1][2];
	r32 b20 = b[2][0], b21 = b[2][1], b22 = b[2][2];

	out[0][0] = a00*b00 + a01*b10 + a02*b20;
	out[0][1] = a00*b01 + a01*b11 + a02*b21;
	out[0][2] = a00*b02 + a01*b12 + a02*b22;

	out[1][0] = a10*b00 + a11*b10 + a12*b20;
	out[1][1] = a10*b01 + a11*b11 + a12*b21;
	out[1][2] = a10*b02 + a11*b12 + a12*b22;

	out[2][0] = a20*b00 + a21*b10 + a22*b20;
	out[2][1] = a20*b01 + a21*b11 + a22*b21;
	out[2][2] = a20*b02 + a21*b12 + a22*b22;
}

void Mat4x4Mul(r32 out[4][4], const r32 a[4][4], const r32 b[4][4]) {
	r32 a00 = a[0][0], a01 = a[0][1], a02 = a[0][2], a03 = a[0][3];
	r32 a10 = a[1][0], a11 = a[1][1], a12 = a[1][2], a13 = a[1][3];
	r32 a20 = a[2][0], a21 = a[2][1], a22 = a[2][2], a23 = a[2][3];
	r32 a30 = a[3][0], a31 = a[3][1], a32 = a[3][2], a33 = a[3][3];

	r32 b00 = b[0][0], b01 = b[0][1], b02 = b[0][2], b03 = b[0][3];
	r32 b10 = b[1][0], b11 = b[1][1], b12 = b[1][2], b13 = b[1][3];
	r32 b20 = b[2][0], b21 = b[2][1], b22 = b[2][2], b23 = b[2][3];
	r32 b30 = b[3][0], b31 = b[3][1], b32 = b[3][2], b33 = b[3][3];

	out[0][0] = a00*b00 + a01*b10 + a02*b20 + a03*b30;
	out[0][1] = a00*b01 + a01*b11 + a02*b21 + a03*b31;
	out[0][2] = a00*b02 + a01*b12 + a02*b22 + a03*b32;
	out[0][3] = a00*b03 + a01*b13 + a02*b23 + a03*b33;

	out[1][0] = a10*b00 + a11*b10 + a12*b20 + a13*b30;
	out[1][1] = a10*b01 + a11*b11 + a12*b21 + a13*b31;
	out[1][2] = a10*b02 + a11*b12 + a12*b22 + a13*b32;
	out[1][3] = a10*b03 + a11*b13 + a12*b23 + a13*b33;

	out[2][0] = a20*b00 + a21*b10 + a22*b20 + a23*b30;
	out[2][1] = a20*b01 + a21*b11 + a22*b21 + a23*b31;
	out[2][2] = a20*b02 + a21*b12 + a22*b22 + a23*b32;
	out[2][3] = a20*b03 + a21*b13 + a22*b23 + a23*b33;

	out[3][0] = a30*b00 + a31*b10 + a32*b20 + a33*b30;
	out[3][1] = a30*b01 + a31*b11 + a32*b21 + a33*b31;
	out[3][2] = a30*b02 + a31*b12 + a32*b22 + a33*b32;
	out[3][3] = a30*b03 + a31*b13 + a32*b23 + a33*b33;
}

