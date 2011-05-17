/*
 * Copyright (C) 2003-2011 Victor Semionov
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  * Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the copyright holder nor the names of the contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Tensor.h"







double VecDot(Vector a, Vector b)
{
	return (a[0]*b[0]+a[1]*b[1]+a[2]*b[2]);
}





void VecCross(Vector a, Vector b, Vector c)
{
	c[0]=a[1]*b[2]-a[2]*b[1];
	c[1]=a[2]*b[0]-a[0]*b[2];
	c[2]=a[0]*b[1]-a[1]*b[0];
}





void MatVecMult(Matrix m, Vector v, Vector r)
{
	r[0]=m[0][0]*v[0]+m[0][1]*v[1]+m[0][2]*v[2];
	r[1]=m[1][0]*v[0]+m[1][1]*v[1]+m[1][2]*v[2];
	r[2]=m[2][0]*v[0]+m[2][1]*v[1]+m[2][2]*v[2];
}





void MatMult(Matrix a, Matrix b, Matrix c)
{
	int i,j,k;
	for (i=0;i<3;i++)
	{
		for (j=0;j<3;j++)
		{
			c[i][j]=0.0f;
			for (k=0;k<3;k++)
			{
				c[i][j]+=a[i][k]*b[k][j];
			}
		}
	}
}





void Matrix3D(float m4d[16], Matrix m3d)
{
	//in opengl the matrix is stored in column-major order!
	m3d[0][0]=m4d[0];
	m3d[0][1]=m4d[4];
	m3d[0][2]=m4d[8];
	m3d[1][0]=m4d[1];
	m3d[1][1]=m4d[5];
	m3d[1][2]=m4d[9];
	m3d[2][0]=m4d[2];
	m3d[2][1]=m4d[6];
	m3d[2][2]=m4d[10];
}
