
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
