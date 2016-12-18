#include "shared.h"

void MatrixMultiply(Vec4 (*lhs)[4], Vec4 (*rhs)[4], Vec4 (*result)[4]) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			r32 sum = 0.0f;
			for (int k = 0; k < 4; ++k) {
				sum += (*lhs)[i][k] * (*rhs)[k][j];
			}
			(*result)[i][j] = sum;
		}
	}
}

void MatrixMultiply(Vec3 (*lhs)[3], Vec3 (*rhs)[3], Vec3 (*result)[3]) {
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			r32 sum = 0.0f;
			for (int k = 0; k < 3; ++k) {
				sum += (*lhs)[i][k] * (*rhs)[k][j];
			}
			(*result)[i][j] = sum;
		}
	}
}

void MatrixMultiply(Vec4 *lhs, Vec4 (*rhs)[4], Vec4 *result) {
	for (int i = 0; i < 4; ++i) {
		r32 sum = 0.0f;
		for (int j = 0; j < 4; ++j) {
			sum += (*lhs)[j] * (*rhs)[j][i];
		}
		(*result)[i] = sum;
	}
}
