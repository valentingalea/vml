#include <cstdio>

#ifdef C4DROID
#define throw(...) // stupid fix for C++17 on c4droid
#endif

#include "swizzle/vector.h"
#include "swizzle/matrix.h"

using vec4 = swizzle::vector<float, 4>;
using vec3 = swizzle::vector<float, 3>;
using vec2 = swizzle::vector<float, 2>;
using ivec4 = swizzle::vector<int, 4>;
using ivec3 = swizzle::vector<int, 3>;
using ivec2 = swizzle::vector<int, 2>;

template<typename T, size_t N, size_t M>
using matrix = swizzle::matrix<swizzle::vector, T, N, M>;
using mat2 = matrix<float, 2, 2>;
using mat3 = matrix<float, 3, 3>;

int main ()
{
	auto v = vec3(1.f, 0.f, 0.f);

	auto d = dot(v.xzz, v.zxz);
	d = dot(v.xzz, v);

	v += d;
	v *= v.yyy;

	auto n = v + v.zzz;

	auto m = mat3(1.f);
	m[0].xxx;

	return 0;
}