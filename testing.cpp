#define CATCH_CONFIG_MAIN // https://github.com/catchorg/Catch2/blob/master/docs/configuration.md#main-implementation
#define CATCH_CONFIG_FAST_COMPILE // https://github.com/catchorg/Catch2/blob/master/docs/configuration.md#catch_config_fast_compile
#include "catch.hpp"

#include "vector.h"

typedef vmath::vector<double, 4> dvec4;
typedef vmath::vector<double, 3> dvec3;
typedef vmath::vector<double, 2> dvec2;
typedef vmath::vector<float, 4> vec4;
typedef vmath::vector<float, 3> vec3;
typedef vmath::vector<float, 2> vec2;
typedef vmath::vector<int, 4> ivec4;
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

	SECTION("combo") {
		ivec4 v = ivec4(1, ivec2(2, 3), 4);
		REQUIRE(v.x == 1);
		REQUIRE(v.y == 2);
		REQUIRE(v.z == 3);
		REQUIRE(v.w == 4);
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

TEST_CASE("spec::Par_5_4_2__Constructors")
{
	int _int = 1;
	float _float = 2;
	vec2 _vec2(3.0f, 4.0f);
	vec3 _vec3(5.0f, 6.0f, 7.0f);
	vec4 _vec4(8.0f, 9.0f, 10.0f, 11.0f);
	vec4 _ivec4(12, 13, 14, 15);
	double _double = 16;
	dvec2 _dvec2(17, 18);
	dvec3 _dvec3(19, 20, 21);
	dvec4 _dvec4(22, 23, 24, 25);

	(vec3(_float)); // initializes each component of the vec3 with the float
	(vec4(_ivec4)); // makes a vec4 with component-wise conversion
					// (vec4(_mat2)); // the vec4 is column 0 followed by column 1 <-- THIS DOES NOT WORK
	vec2(_float, _float); // initializes a vec2 with 2 floats
	ivec3(_int, _int, _int); // initializes an ivec3 with 3 ints
	(vec2(_vec3)); // drops the third component of a vec3
	(vec3(_vec4)); // drops the fourth component of a vec4
	vec3(_vec2, _float); // vec3.x = vec2.x, vec3.y = vec2.y, vec3.z = float
	vec3(_float, _vec2); // vec3.x = float, vec3.y = vec2.x, vec3.z = vec2.y
	vec4(_vec3, _float);
	vec4(_float, _vec3);
	vec4(_vec2, _vec2);

	vec4 color = vec4(0.0, 1.0, 0.0, 1.0);
	vec4 rgba = vec4(1.0); // sets each component to 1.0
	vec3 rgb = vec3(color); // drop the 4th component

	REQUIRE(rgb.x == Approx(0.f));
	REQUIRE(rgb.y == Approx(1.f));
	REQUIRE(rgb.z == Approx(0.f));
}
