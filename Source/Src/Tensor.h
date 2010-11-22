


typedef	double	Vector[3];
typedef	double	Matrix[3][3];


extern	double	VecDot(Vector a, Vector b);
extern	void	VecCross(Vector a, Vector b, Vector c);
extern	void	MatVecMult(Matrix m, Vector v, Vector r);
extern	void	MatMult(Matrix a, Matrix b, Matrix c);
extern	void	Matrix3D(float m4d[16], Matrix m3d);
