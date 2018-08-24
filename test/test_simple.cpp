#include <cstdio>

#include "c4droid.h"

#include "../vml/vector.h"
#include "../vml/matrix.h"

using  vec4 = vml::vector<float, 0, 1, 2, 3>;
using  vec3 = vml::vector<float, 0, 1, 2>;
using  vec2 = vml::vector<float, 0, 1>;
using ivec4 = vml::vector<  int, 0, 1, 2, 3>;
using ivec3 = vml::vector<  int, 0, 1, 2>;
using ivec2 = vml::vector<  int, 0, 1>;

using mat2 = vml::matrix<float, vml::vector, 2, 2>;
using mat3 = vml::matrix<float, vml::vector, 3, 3>;

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

	mat2 m
		//(1, 2, 3, 4);
		(vec2(1, 2), vec2(3, 4));

	return 0;
}