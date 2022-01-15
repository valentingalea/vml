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
	vec4 a = vec4(1.f, 2.f, 3.f, 4.f);
	vec4 b = a.wxyx * 2.f;
	dot(a, b);

	return 0;
}