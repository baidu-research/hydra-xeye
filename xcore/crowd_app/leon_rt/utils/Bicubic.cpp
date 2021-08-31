// Bicubic interpolation.cpp : 定义控制台应用程序的入口点。
//
#if 1

float cubicInterpolate(float p[4], float x)
{
	return p[1] + 0.5 * x*(p[2] - p[0] + x*(2.0*p[0] - 5.0*p[1] + 4.0*p[2] - p[3] + x*(3.0*(p[1] - p[2]) + p[3] - p[0])));
}

float bicubicInterpolate(float p[4][4], float x, float y)
{
	float arr[4];
	arr[0] = cubicInterpolate(p[0], y);
	arr[1] = cubicInterpolate(p[1], y);
	arr[2] = cubicInterpolate(p[2], y);
	arr[3] = cubicInterpolate(p[3], y);
	return cubicInterpolate(arr, x);
}

float tricubicInterpolate(float p[4][4][4], float x, float y, float z)
{
	float arr[4];
	arr[0] = bicubicInterpolate(p[0], y, z);
	arr[1] = bicubicInterpolate(p[1], y, z);
	arr[2] = bicubicInterpolate(p[2], y, z);
	arr[3] = bicubicInterpolate(p[3], y, z);
	return cubicInterpolate(arr, x);
}

float nCubicInterpolate(int n, float* p, float coordinates[])
{
	//assert(n > 0);
	if (n == 1)
	{
		return cubicInterpolate(p, *coordinates);
	}
	else
	{
		float arr[4];
		int skip = 1 << (n - 1) * 2;
		arr[0] = nCubicInterpolate(n - 1, p, coordinates + 1);
		arr[1] = nCubicInterpolate(n - 1, p + skip, coordinates + 1);
		arr[2] = nCubicInterpolate(n - 1, p + 2 * skip, coordinates + 1);
		arr[3] = nCubicInterpolate(n - 1, p + 3 * skip, coordinates + 1);
		return cubicInterpolate(arr, *coordinates);
	}
}
#if 0
int mainlllll()
{
	// Create array
	float p[4][4] = { { 1,3,3,4 },{ 7,2,3,4 },{ 1,6,3,6 },{ 2,5,7,2 } };

	// Interpolate
	std::cout << bicubicInterpolate(p, 0.1, 0.2) << '\n';
	std::cout << bicubicInterpolate(p, 0.5, 0.5) << '\n';
	std::cout << bicubicInterpolate(p, 1, 1) << '\n';
	std::cout << bicubicInterpolate(p, 0, 0) << '\n';

	// Or use the nCubicInterpolate function
	//float co[2] = { 0.1, 0.2 };
	//std::cout << nCubicInterpolate(2, (float*)p, co) << '\n';
	
	//system("pause");
	return 0;
}
#endif
#endif
