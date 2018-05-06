template<typename T, int N>
struct vector_base
{
	union
	{
		T data[N];
	};
};

template<typename T>
struct vector_base<T, 2>
{
	union
	{
		T data[2];
		struct { T x, y; };
		struct { T s, t; };
		struct { T u, v; };
		swizzler<T, 2, 0, 0> xx;
		swizzler<T, 2, 0, 1> xy;
		swizzler<T, 2, 1, 0> yx;
		swizzler<T, 2, 1, 1> yy;
	};
};

template<typename T>
struct vector_base<T, 3>
{
	union
	{
		T data[3];
		struct { T x, y, z; };
		struct { T r, g, b; };
		swizzler<T, 2, 0, 0> xx;
		swizzler<T, 2, 0, 1> xy;
		swizzler<T, 2, 0, 2> xz;
		swizzler<T, 2, 1, 0> yx;
		swizzler<T, 2, 1, 1> yy;
		swizzler<T, 2, 1, 2> yz;
		swizzler<T, 2, 2, 0> zx;
		swizzler<T, 2, 2, 1> zy;
		swizzler<T, 2, 2, 2> zz;
		swizzler<T, 3, 0, 0, 0> xxx;
		swizzler<T, 3, 0, 0, 1> xxy;
		swizzler<T, 3, 0, 0, 2> xxz;
		swizzler<T, 3, 0, 1, 0> xyx;
		swizzler<T, 3, 0, 1, 1> xyy;
		swizzler<T, 3, 0, 1, 2> xyz;
		swizzler<T, 3, 0, 2, 0> xzx;
		swizzler<T, 3, 0, 2, 1> xzy;
		swizzler<T, 3, 0, 2, 2> xzz;
		swizzler<T, 3, 1, 0, 0> yxx;
		swizzler<T, 3, 1, 0, 1> yxy;
		swizzler<T, 3, 1, 0, 2> yxz;
		swizzler<T, 3, 1, 1, 0> yyx;
		swizzler<T, 3, 1, 1, 1> yyy;
		swizzler<T, 3, 1, 1, 2> yyz;
		swizzler<T, 3, 1, 2, 0> yzx;
		swizzler<T, 3, 1, 2, 1> yzy;
		swizzler<T, 3, 1, 2, 2> yzz;
		swizzler<T, 3, 2, 0, 0> zxx;
		swizzler<T, 3, 2, 0, 1> zxy;
		swizzler<T, 3, 2, 0, 2> zxz;
		swizzler<T, 3, 2, 1, 0> zyx;
		swizzler<T, 3, 2, 1, 1> zyy;
		swizzler<T, 3, 2, 1, 2> zyz;
		swizzler<T, 3, 2, 2, 0> zzx;
		swizzler<T, 3, 2, 2, 1> zzy;
		swizzler<T, 3, 2, 2, 2> zzz;
	};
};

template<typename T>
struct vector_base<T, 4>
{
	union
	{
		T data[4];
		struct { T x, y, z, w; };
		struct { T r, g, b, a; };
	};
};