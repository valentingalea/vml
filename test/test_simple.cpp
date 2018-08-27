#include <cstdio>

#include "c4droid.h"

#include "../vml/vector.h"
#include "../vml/matrix.h"
#include "../vml/vector_functions.h"

using  vec4 = vml::vector<float, 0, 1, 2, 3>;
using  vec3 = vml::vector<float, 0, 1, 2>;
using  vec2 = vml::vector<float, 0, 1>;
using   _01 = vml::indices_pack<0, 1>;
using  _012 = vml::indices_pack<0, 1, 2>;
using _0123 = vml::indices_pack<0, 1, 2, 3>;
using  mat2 = vml::matrix<float, vml::vector, _01, _01>;
using  mat3 = vml::matrix<float, vml::vector, _012, _012>;
using  mat4 = vml::matrix<float, vml::vector, _0123, _0123>;



int main ()
{
	using bvec3 = vml::vector<bool, 0, 1, 2>;
	bvec3 b;
	auto bb = any(b);

	auto v = vec3(42.f);

	float q = radians(3.14);

	v *= 1.f;
	v *= v.bbb;

	dot(v.xx, v.zz);

	auto mm = mat2(1, 2, 3, 4);
	auto mmm = mat3(0, vec2(1, 2), vec2(3, 4), 5, v.xyx);
	mmm * mmm;
	mm *= 1.f;

	return 0;
}