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