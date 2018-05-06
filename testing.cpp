#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "vector.h"

typedef vmath::vector<float, 3> vec3;
typedef vmath::vector<float, 2> vec2;
typedef vmath::vector<int, 3> ivec3;
typedef vmath::vector<int, 2> ivec2;

TEST_CASE("vec2 basic init", "[vec2]")
{
	SECTION("default ctor") {
		vec2 v;
		REQUIRE(v.x == 0.f);
		REQUIRE(v.y == 0.f);
	}

	SECTION("scalar ctor") {
		ivec2 v(41);
		v = 42;
		REQUIRE(v.x == 42);
		REQUIRE(v.y == 42);
	}

	//TODO: doesn't work because of explicit ctor
	//SECTION("via init list") {
	//	ivec2 v = { 3, 4 };
	//	REQUIRE(v.x == 3);
	//	REQUIRE(v.y == 4);
	//}

	SECTION("direct init") {
		ivec2 v = ivec2(3, 4);
		REQUIRE(v.x == 3);
		REQUIRE(v.y == 4);
	}

	SECTION("decay") {
		ivec2 v = ivec2(3.14);
		REQUIRE(v.x == 3);
	}

//
// SHOULDN'T COMPILE
//
	//ivec2 v = ivec2(1, 2, 3);

	//struct dummy {} d;
	//ivec2 v = ivec2(d);
}

TEST_CASE("swizzle construct", "[vec2][vec3]")
{
	ivec3 v(1, 2, 3);

	SECTION("swizzle reverse") {
		ivec3 rev = v.zyx;
		REQUIRE(rev.x == 3);
		REQUIRE(rev.y == 2);
		REQUIRE(rev.z == 1);
	}

	SECTION("swizzle repeat") {
		ivec3 rep = v.zzz;
		REQUIRE(rep.x == 3);
		REQUIRE(rep.y == 3);
		REQUIRE(rep.z == 3);
	}

	//TODO: doesn't work - swizzler type doesn't convert properly to parent type
	//SECTION("swizzle chain") {
	//	ivec3 rep = v.zzz.xyz;
	//	REQUIRE(rep.x == 3);
	//	REQUIRE(rep.y == 3);
	//	REQUIRE(rep.z == 3);
	//}
	//SECTION("swizzle implicit ctor") {
	//	ivec3 rep(v.xy, 42);
	//	REQUIRE(rep.x == 1);
	//	REQUIRE(rep.y == 2);
	//	REQUIRE(rep.z == 42);
	//}

	SECTION("swizzle explicit ctor") {
		ivec3 rep(ivec2(3, 4), 5);
		REQUIRE(rep.x == 3);
		REQUIRE(rep.y == 4);
		REQUIRE(rep.z == 5);
	}
}

TEST_CASE("lvalues", "[vec2][vec3]")
{
	ivec3 _3(101, 102, 103);
	ivec2 _2;

	SECTION("vec2 from vec3 swizzle") {
		_2.xy = _3.xy;
		REQUIRE(_2.x == 101);
		REQUIRE(_2.y == 102);
	}

	//TODO: doesn't work - see above comments
	//SECTION("vec3 swizzle from vec2") {
	//	_3.xy = _2;
	//	REQUIRE(_3.x == 0);
	//	REQUIRE(_3.y == 0);
	//}
	//SECTION("vec3 from single component") {
	//	_3.xyz = _2.x;
	//}
}

TEST_CASE("operators", "[vec2]")
{
	vec2 uv;
	vec2 p = 2.f * uv - 1.f;
	REQUIRE(p.x == Approx(-1.f));
	REQUIRE(p.y == Approx(-1.f));
}