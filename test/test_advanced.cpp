#ifdef C4DROID
// stupid fixes for C++17 on C4droid
#define _SILENCE_CXX17_UNCAUGHT_EXCEPTION_DEPRECATION_WARNING
#define CATCH_CONFIG_NO_CPP17_UNCAUGHT_EXCEPTIONS
#define throw(...)
#endif

#define CATCH_CONFIG_MAIN // https://github.com/catchorg/Catch2/blob/master/docs/configuration.md#main-implementation
#define CATCH_CONFIG_FAST_COMPILE // https://github.com/catchorg/Catch2/blob/master/docs/configuration.md#catch_config_fast_compile
#include "catch.hpp"

#ifdef _MSC_VER
// turn off for the purposes of these tests
// warning C4244: 'argument': conversion from 'double' to 'int', possible loss of data
#pragma warning(disable: 4244)
#endif

#include "swizzle/vector.h"
//#include "swizzle/matrix.h"

using dvec4 = vml::vector<double, 0, 1, 2, 3>;
using dvec3 = vml::vector<double, 0, 1, 2>;
using dvec2 = vml::vector<double, 0, 1>;
using  vec4 = vml::vector< float, 0, 1, 2, 3>;
using  vec3 = vml::vector< float, 0, 1, 2>;
using  vec2 = vml::vector< float, 0, 1>;
using ivec4 = vml::vector<   int, 0, 1, 2, 3>;
using ivec3 = vml::vector<   int, 0, 1, 2>;
using ivec2 = vml::vector<   int, 0, 1>;

static_assert(sizeof(vec4) == (sizeof(float) * 4), "vec4 size mismatch");
static_assert(sizeof(vec3) == (sizeof(float) * 3), "vec3 size mismatch");
static_assert(sizeof(vec2) == (sizeof(float) * 2), "vec2 size mismatch");

//typedef vml::matrix< vml::vector, float, 2, 2> mat2;
//typedef vml::matrix< vml::vector, float, 3, 3> mat3;
//typedef vml::matrix< vml::vector, float, 4, 4> mat4;
//typedef mat2 mat2x2;
//typedef mat3 mat3x3;
//typedef mat4 mat4x4;
//typedef vml::matrix< vml::vector, float, 3, 2> mat3x2;
//typedef vml::matrix< vml::vector, float, 4, 2> mat4x2;
//typedef vml::matrix< vml::vector, float, 2, 3> mat2x3;
//typedef vml::matrix< vml::vector, float, 3, 4> mat3x4;
//typedef vml::matrix< vml::vector, double, 2, 2> dmat2;
//typedef vml::matrix< vml::vector, double, 3, 3> dmat3;
//typedef vml::matrix< vml::vector, double, 4, 4> dmat4;
//typedef vml::matrix< vml::vector, double, 2, 4> dmat2x4;
//// not GLSL but for easier testing
//typedef vml::matrix< vml::vector, int, 2, 2> imat2;
//typedef vml::matrix< vml::vector, int, 3, 3> imat3;

TEST_CASE("vec2 basic init", "[vec2]")
{
	SECTION("default ctor") {
		vec2 v;
		REQUIRE(v.x == 0.f);
		REQUIRE(v.y == 0.f);
	}

	SECTION("scalar ctor") {
		ivec2 v(41);
		REQUIRE(v.x == 41);
		REQUIRE(v.y == 41);
	}

//TODO: implement
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
		REQUIRE(v.y == 3);
	}

	SECTION("combo init same type") {
		ivec4 v = ivec4(1, ivec2(2, 3), 4);
		REQUIRE(v.x == 1);
		REQUIRE(v.y == 2);
		REQUIRE(v.z == 3);
		REQUIRE(v.w == 4);
	}

	SECTION("combo init diff type") {
		vec4 v = vec4(1, ivec2(2, 3), 4);
		REQUIRE(v.x == Approx(1.f));
		REQUIRE(v.y == Approx(2.f));
		REQUIRE(v.z == Approx(3.f));
		REQUIRE(v.w == Approx(4.f));
	}

//TODO: fix this
	//SECTION("over limit") {
	//	ivec2 _2 = ivec2(0, 0);
	//	ivec3 _3 = ivec3(0, 0, 1);
	//	ivec4 _4 = ivec4(_2, _3);
	//	REQUIRE(_4.x == 0);
	//	REQUIRE(_4.y == 0);
	//	REQUIRE(_4.z == 0);
	//	REQUIRE(_4.w == 0);
	//}
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

//TODO: can this ever work?
	//SECTION("swizzle chain") {
	//	ivec3 rep = v.zzz.xyz;
	//	REQUIRE(rep.x == 3);
	//	REQUIRE(rep.y == 3);
	//	REQUIRE(rep.z == 3);
	//}

	SECTION("swizzle implicit ctor") {
		ivec3 rep(v.xy, 42);
		REQUIRE(rep.x == 1);
		REQUIRE(rep.y == 2);
		REQUIRE(rep.z == 42);
	}
}

TEST_CASE("lvalues", "[vec2][vec3]")
{
	ivec3 _3(101, 102, 103);
	ivec2 _2(11, 12);

	SECTION("vec2 from vec3 swizzle") {
		_2.xy = _3.xy;
		REQUIRE(_2.x == 101);
		REQUIRE(_2.y == 102);
	}

	SECTION("vec3 swizzle from vec2") {
		_3.xy = _2;
		REQUIRE(_3.x == 11);
		REQUIRE(_3.y == 12);
	}
	SECTION("vec3 from single component") {
		_3.xyz = ivec3(_2.x);
		REQUIRE(_3.x == 11);
		REQUIRE(_3.y == 11);
		REQUIRE(_3.z == 11);
	}
}

//TEST_CASE("matrix ctor")
//{
//	SECTION("identity") {
//		auto m = imat2(1);
//		REQUIRE(m[0].x == 1);
//		REQUIRE(m[0].y == 0);
//		REQUIRE(m[1].x == 0);
//		REQUIRE(m[1].y == 1);
//	}
//
//	SECTION("from scalars") {
//		auto m = imat2(1, 2, 3, 4);
//		REQUIRE(m[0].x == 1);
//		REQUIRE(m[0].y == 2);
//		REQUIRE(m[1].x == 3);
//		REQUIRE(m[1].y == 4);
//	}
//
//	SECTION("from 2 vec") {
//		auto m = imat2(ivec2(1, 2), ivec2(3, 4));
//		REQUIRE(m[0].x == 1);
//		REQUIRE(m[0].y == 2);
//		REQUIRE(m[1].x == 3);
//		REQUIRE(m[1].y == 4);
//	}
//
//	SECTION("from mix") {
//		auto v = ivec3(2, 3, 4);
//		auto m = imat2(1, v.xyz);
//		REQUIRE(m[0].x == 1);
//		REQUIRE(m[0].y == 2);
//		REQUIRE(m[1].x == 3);
//		REQUIRE(m[1].y == 4);
//	}
//}

TEST_CASE("operators", "[vec2]")
{
	vec2 uv;
	vec2 p = 2.f * uv - 1.f;
	REQUIRE(p.x == Approx(-1.f));
	REQUIRE(p.y == Approx(-1.f));
}

TEST_CASE("builtin functions")
{
	vec3 v = vec3(1.f, 0.f, 0.f);
	REQUIRE(length(v) == Approx(1.f));

	float d = dot(v.xzz, v.zxz);
	REQUIRE(d == Approx(0.f));
	d = dot(v.xzz, v);
	REQUIRE(d == Approx(1.f));

	vec3 c = cross(v.xzz, v.zzx);
	REQUIRE(c.x == Approx(0.f));
	REQUIRE(c.y == Approx(-1.f));
	REQUIRE(c.z == Approx(0.f));
}

TEST_CASE("union member access")
{
	vec2 v;

	v.x = 42;
	v.y = 43;
	v.r = 44;

	REQUIRE(v.x == 44);
	REQUIRE(v[1] == 43);

	v.xy.data[0] = 99;
	REQUIRE(v.x == 99);
}

void inout_func(vec3 &) {}

TEST_CASE("inout args")
{
	auto v = vec3(0);
	inout_func(v);

	//TODO: https://github.com/gwiazdorrr/CxxSwizzle/issues/4
	//inout_func(v.xyz);
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
	//mat2 _mat2(26, 27, 28, 29);

	//mat4x4 _mat4x4;
	//mat4x2 _mat4x2;
	//mat3x3 _mat3x3;

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

	//(mat2(_float));
	//(mat3(_float));
	//(mat4(_float));

	//mat2(_vec2, _vec2); // one column per argument
	//mat3(_vec3, _vec3, _vec3); // one column per argument
	//mat4(_vec4, _vec4, _vec4, _vec4); // one column per argument
	//mat3x2(_vec2, _vec2, _vec2); // one column per argument
	//dmat2(_dvec2, _dvec2);
	//dmat3(_dvec3, _dvec3, _dvec3);
	//dmat4(_dvec4, _dvec4, _dvec4, _dvec4);
	//mat2(_float, _float, // first column
	//	_float, _float); // second column
	//mat3(_float, _float, _float, // first column
	//	_float, _float, _float, // second column
	//	_float, _float, _float); // third column
	//mat4(_float, _float, _float, _float, // first column
	//	_float, _float, _float, _float, // second column
	//	_float, _float, _float, _float, // third column
	//	_float, _float, _float, _float); // fourth column
	//mat2x3(_vec2, _float, // first column
	//	_vec2, _float); // second column
	//dmat2x4(_dvec3, _double, // first column
	//	_double, _dvec3); // second column

	REQUIRE(rgb.x == Approx(0.f));
	REQUIRE(rgb.y == Approx(1.f));
	REQUIRE(rgb.z == Approx(0.f));
}

TEST_CASE("spec::Par_5_5__Vector_and_Scalar_Components_and_Length")
{
	{
		vec2 pos;
		//float height;
		pos.x; // is legal
			   // pos.z // is illegal
			   // height.x; // is legal  <-- THIS DOES NOT WORK
			   // height.y // is illegal
	}

	{
		vec4 v4;
		v4.rgba; // is a vec4 and the same as just using v4,
		v4.rgb; // is a vec3,
		v4.b; // is a float,
		v4.xy; // is a vec2,
			   // v4.xgba; // is illegal - the component names do not come from 
	}

	{
		vec4 pos = vec4(1.0, 2.0, 3.0, 4.0);
		vec4 swiz = pos.wzyx;
		REQUIRE(swiz.x == Approx(4.f));
		REQUIRE(swiz.y == Approx(3.f));
		REQUIRE(swiz.z == Approx(2.f));
		REQUIRE(swiz.w == Approx(1.f));
		vec4 dup = pos.xxyy;
		REQUIRE(dup.x == Approx(1.f));
		REQUIRE(dup.y == Approx(1.f));
		REQUIRE(dup.z == Approx(2.f));
		REQUIRE(dup.w == Approx(2.f));
		//float f = 1.2;
		// vec4 dup = f.xxxx; // dup = (1.2, 1.2, 1.2, 1.2) <-- THIS DOES NOT WORK
	}

	{
		vec4 pos = vec4(1.0, 2.0, 3.0, 4.0);
		pos.xw = vec2(5.0, 6.0);
		pos.wx = vec2(7.0, 8.0);
		//pos.xx = vec2(3.0, 4.0); // illegal - 'x' used twice
		//pos.xy = vec3(1.0, 2.0, 3.0); // illegal - mismatch between vec2 and vec3
	}

	{
		//vec3 v;
		// const int L = v.length();  <-- THIS DOES NOT WORK
	}
}