#include <cstdio>

#include "c4droid.h"

#include "../vml/vector.h"
//#include "swizzle/matrix.h"

using  vec4 = vml::vector<float, 0, 1, 2, 3>;
using  vec3 = vml::vector<float, 0, 1, 2>;
using  vec2 = vml::vector<float, 0, 1>;
using ivec4 = vml::vector<  int, 0, 1, 2, 3>;
using ivec3 = vml::vector<  int, 0, 1, 2>;
using ivec2 = vml::vector<  int, 0, 1>;

//template<typename T, size_t N, size_t M>
//using matrix = vml::matrix<vml::vector, T, N, M>;
//using mat2 = matrix<float, 2, 2>;
//using mat3 = matrix<float, 3, 3>;

int main ()
{
	auto n = sizeof(vec3);
	auto v = vec3(42.f);// , 0.f, 0.f);
	v += 2.f;
	dot(v.xx, v.zz);

	//auto d = dot(v.xzz, v.zxz);
	//d = dot(v.xzz, v);

	//v += d;
	//v *= v.yyy;

	//auto n = v + v.zzz;

	//auto m = mat3(1.f);
	//m[0].xxx;

	return 0;
}