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

using  _01 = vml::indices_pack<0, 1>;
using _012 = vml::indices_pack<0, 1, 2>;
using mat2 = vml::matrix<float, vml::vector, _01, _01>;
using mat3 = vml::matrix<float, vml::vector, _012, _012>;
using mat3x2 = vml::matrix<float, vml::vector, _012, _01>;

int main ()
{
	auto v = vec3(42.f);

	v *= v.bbb;

	dot(v.xx, v.zz);

	auto mm = mat2(1, 2, 3, 4);
	auto mmm = mat3(0, vec2(1, 2), vec2(3, 4), 5, v.xyx);
	mmm * mmm;

	return 0;
}