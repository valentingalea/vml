#include <cstdio>

// stupid fix for C++17 on c4droid
#define throw(...)

#include "swizzle/vector.h"

typedef swizzle::vector<float, 4> vec4;
typedef swizzle::vector<float, 3> vec3;
typedef swizzle::vector<float, 2> vec2;
typedef swizzle::vector<int, 4> ivec4;
typedef swizzle::vector<int, 3> ivec3;
typedef swizzle::vector<int, 2> ivec2;

int main ()
{
	auto v = vec3(1, vec2(2, 3));
	vec2 s = v.xx;
	v.xy = s;

	return 0;
}