/*
 *  MatrixMath.h Library for Matrix Math
 *
 *  Created by Charlie Matlack on 12/18/10.
 *  Modified from code by RobH45345 on Arduino Forums, taken from unknown source.
 */

#ifndef MatrixMath_h
#define MatrixMath_h

#include "Marlin.h"

class MatrixMath
{
public:
	MatrixMath();
	void MatrixPrint(float* A, int m, int n, String label);
	void MatrixCopy(float* A, int n, int m, float* B);
	void MatrixMult(float* A, float* B, int m, int p, int n, float* C);
	void MatrixAdd(float* A, float* B, int m, int n, float* C);
	void MatrixSubtract(float* A, float* B, int m, int n, float* C);
	void MatrixTranspose(float* A, int m, int n, float* C);
	int MatrixInvert(float* A, int n);
	void MatrixIdentity(float* A, int m, int n);
};

extern MatrixMath matrixMaths;

#endif