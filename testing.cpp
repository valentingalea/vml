#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "vector.h"

typedef vmath::vector<float, 4> vec4;
typedef vmath::vector<float, 3> vec3;
typedef vmath::vector<float, 2> vec2;

TEST_CASE("basic init", "[spec][vec2]")
{
	vec2 v;

	REQUIRE(v.x == 0.f);
	REQUIRE(v.y == 0.f);
}