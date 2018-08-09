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
	auto axis_x = vec3(1.f, 0.f, 0.f);
	auto axis_y = vec3(0.f, 1.f, 0.f);

	auto d = dot(axis_x, axis_y);

	return 0;
}