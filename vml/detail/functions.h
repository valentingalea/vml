#pragma once

#include <cmath>

namespace vml { namespace detail
{

template<template<typename, size_t...> class vector, typename scalar_type, size_t... Ns>
struct builtin_func_lib
{
#define FUNC(x) lib_##x
#ifdef _MSC_VER
#define LIB static __forceinline
#else
#define LIB static __attribute__((always_inline))
#endif

	using vector_type = vector<scalar_type, Ns...>;
	using vector_arg_type = const vector_type &;
	using bool_vector_type = vector<bool, Ns...>;

	static constexpr auto one = scalar_type(1);
	static constexpr auto zero = scalar_type(0);

//
// 8.1 Angle and Trigonometry Function
//
	LIB vector_type FUNC(radians)(vector_arg_type degrees)
	{
		constexpr auto pi_over_180 = scalar_type(3.14159265358979323846 / 180);
		return vector_type((degrees.data[Ns] * pi_over_180)...);
	}

	LIB vector_type FUNC(degrees)(vector_arg_type radians)
	{
		constexpr auto _180_over_pi = scalar_type(180 / 3.14159265358979323846);
		return vector_type((radians.data[Ns] * _180_over_pi)...);
	}

	LIB vector_type FUNC(sin)(vector_arg_type t)
	{
		return vector_type(std::sin(t.data[Ns])...);
	}

	LIB vector_type FUNC(cos)(vector_arg_type t)
	{
		return vector_type(std::cos(t.data[Ns])...);
	}

	LIB vector_type FUNC(tan)(vector_arg_type t)
	{
		return vector_type(std::tan(t.data[Ns])...);
	}

	LIB vector_type FUNC(asin)(vector_arg_type t)
	{
		return vector_type(std::asin(t.data[Ns])...);
	}

	LIB vector_type FUNC(acos)(vector_arg_type t)
	{
		return vector_type(std::acos(t.data[Ns])...);
	}

	LIB vector_type FUNC(atan)(vector_arg_type y, vector_arg_type x)
	{
		return vector_type(std::atan2(y.data[Ns], x.data[Ns])...);
	}

	LIB vector_type FUNC(atan)(vector_arg_type t)
	{
		return vector_type(std::atan(t.data[Ns])...);
	}

	// TODO: sinh cosh tanh asinh acosh atanh

//
// 8.2 Exponential Functions
//
	LIB vector_type FUNC(pow)(vector_arg_type x, vector_arg_type y)
	{
		return vector_type(std::pow(x.data[Ns], y.data[Ns])...);
	}

	LIB vector_type FUNC(exp)(vector_arg_type t)
	{
		return vector_type(std::exp(t.data[Ns])...);
	}

	LIB vector_type FUNC(log)(vector_arg_type t)
	{
		return vector_type(std::log(t.data[Ns])...);
	}

	LIB vector_type FUNC(exp2)(vector_arg_type t)
	{
		return vector_type(std::exp2(t.data[Ns])...);
	}

	LIB vector_type FUNC(log2)(vector_arg_type t)
	{
		return vector_type(std::log2(t.data[Ns])...);
	}

	LIB vector_type FUNC(sqrt)(vector_arg_type t)
	{
		return vector_type(std::sqrt(t.data[Ns])...);
	}

	LIB scalar_type rsqrt(scalar_type t)
	{
		return one / std::sqrt(t); //NOTE: https://sites.google.com/site/burlachenkok/various_way_to_implement-rsqrtx-in-c
	}

	LIB vector_type FUNC(inversesqrt)(vector_arg_type t)
	{
		return vector_type(rsqrt(t.data[Ns])...);
	}

//
// 8.3 Common Functions
//
	LIB vector_type FUNC(abs)(vector_arg_type t)
	{
		return vector_type(std::abs(t.data[Ns])...);
	}

	LIB scalar_type sign(scalar_type x)
	{
		return (zero < x) - (x < zero);
	}

	LIB vector_type FUNC(sign)(vector_arg_type t)
	{
		return vector_type(sign(t.data[Ns])...);
	}

	LIB vector_type FUNC(floor)(vector_arg_type t)
	{
		return vector_type(std::floor(t.data[Ns])...);
	}

	LIB vector_type FUNC(trunc)(vector_arg_type t)
	{
		return vector_type(std::trunc(t.data[Ns])...);
	}

	//TODO: round roundEven

	LIB vector_type FUNC(ceil)(vector_arg_type t)
	{
		return vector_type(std::ceil(t.data[Ns])...);
	}

	LIB vector_type FUNC(fract)(vector_arg_type t)
	{
		return vector_type((t.data[Ns] - std::floor(t.data[Ns]))...);
	}

	LIB vector_type FUNC(mod)(vector_arg_type x, scalar_type y)
	{
		return vector_type((x.data[Ns] - y * std::floor(x.data[Ns] / y))...);
	}

	LIB vector_type FUNC(mod)(vector_arg_type x, vector_arg_type y)
	{
		return vector_type((x.data[Ns] - y.data[Ns] * std::floor(x.data[Ns] / y.data[Ns]))...);
	}
	
	LIB vector_type FUNC(min)(vector_arg_type left, vector_arg_type right)
	{
		return vector_type((left.data[Ns] < right.data[Ns] ? left.data[Ns] : right.data[Ns])...);
	}

	LIB vector_type FUNC(min)(vector_arg_type left, scalar_type right)
	{
		return vector_type((left.data[Ns] < right ? left.data[Ns] : right)...);
	}

	LIB vector_type FUNC(max)(vector_arg_type left, vector_arg_type right)
	{
		return vector_type((left.data[Ns] > right.data[Ns] ? left.data[Ns] : right.data[Ns])...);
	}

	LIB vector_type FUNC(max)(vector_arg_type left, scalar_type right)
	{
		return vector_type((left.data[Ns] > right ? left.data[Ns] : right)...);
	}

	LIB vector_type FUNC(clamp)(vector_arg_type x, vector_arg_type minVal, vector_arg_type maxVal)
	{
		return FUNC(min)(FUNC(max)(x, minVal), maxVal);
	}

	LIB vector_type FUNC(clamp)(vector_arg_type x, scalar_type minVal, scalar_type maxVal)
	{
		return FUNC(min)(FUNC(max)(x, minVal), maxVal);
	}

	LIB vector_type FUNC(mix)(vector_arg_type x, vector_arg_type y, vector_arg_type a)
	{
		return x * (vector_type(scalar_type(1)) - a) + y * a;
	}

	LIB vector_type FUNC(mix)(vector_arg_type x, vector_arg_type y, scalar_type a)
	{
		return x * (scalar_type(1) - a) + y * a;
	}

	// doens't work in MSVC
	//LIB vector_type FUNC(mix)(vector_arg_type x, vector_arg_type y, const bool_vector_type &a)
	//{
	//	return vector_type((a.data[Ns] ? y.data[Ns] : x.data[Ns])...);
	//}

	LIB vector_type FUNC(step)(scalar_type edge, vector_arg_type x)
	{
		return vector_type((x.data[Ns] < edge ? scalar_type(0) : scalar_type(1))...);
	}

	LIB vector_type FUNC(step)(vector_arg_type edge, vector_arg_type x)
	{
		return vector_type((x.data[Ns] < edge.data[Ns] ? scalar_type(0) : scalar_type(1))...);
	}

	LIB vector_type FUNC(smoothstep)(scalar_type edge0, scalar_type edge1, vector_arg_type x)
	{
		auto t = FUNC(clamp)((x - edge0) / (edge1 - edge0), zero, one);
		return t * t * (scalar_type(3) - scalar_type(2) * t);
	}

	LIB vector_type FUNC(smoothstep)(vector_arg_type edge0, vector_arg_type edge1, vector_arg_type x)
	{
		auto t = FUNC(clamp)((x - edge0) / (edge1 - edge0), zero, one);
		return t * t * (scalar_type(3) - scalar_type(2) * t);
	}
//
// 8.5 Geometric functions
//
	LIB scalar_type FUNC(length)(vector_arg_type v)
	{
		return std::sqrt(FUNC(dot)(v, v));
	}

	LIB scalar_type FUNC(distance)(vector_arg_type p0, vector_arg_type p1)
	{
		return FUNC(length)(p0 - p1);
	}

	LIB vector_type FUNC(normalize)(vector_arg_type v)
	{
		vector_type out = v;
		out /= FUNC(length)(v);
		return out;
	}

	LIB scalar_type FUNC(dot)(vector_arg_type a, vector_arg_type b)
	{
		scalar_type sum = 0;
		((sum += a.data[Ns] * b.data[Ns]), ...);
		return sum;
	}

	LIB vector_type FUNC(cross)(vector_arg_type a, vector_arg_type b)
	{
		static_assert(vector_type::num_components == 3, "cross product only works for vec3");

		return vector_type(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
		);
	}

	LIB vector_type FUNC(faceforward)(vector_arg_type N, vector_arg_type I, vector_arg_type Nref)
	{
		return (FUNC(dot)(Nref, I) < scalar_type(0) ? N : (-N));
	}

	LIB vector_type FUNC(reflect)(vector_arg_type I, vector_arg_type N)
	{
		return (I - scalar_type(2) * FUNC(dot)(I, N) * N);
	}

	LIB vector_type FUNC(refract)(vector_arg_type I, vector_arg_type N, scalar_type eta)
	{
		auto k = one - eta * eta * (one - FUNC(dot)(N, I) * FUNC(dot)(N, I));
		if (k < zero) {
			return vector_type();
		} else {
			return eta * I - (eta * FUNC(dot)(N, I) + sqrt(k)) * N;
		}
	}

//
// 8.7 Vector Relational Functions
//
	LIB bool_vector_type FUNC(lessThan)(vector_arg_type x, vector_arg_type y)
	{
		return bool_vector_type((x.data[Ns] < y.data[Ns])...);
	}

	LIB bool_vector_type FUNC(lessThanEqual)(vector_arg_type x, vector_arg_type y)
	{
		return bool_vector_type((x.data[Ns] <= y.data[Ns])...);
	}

	LIB bool_vector_type FUNC(greaterThan)(vector_arg_type x, vector_arg_type y)
	{
		return bool_vector_type((x.data[Ns] > y.data[Ns])...);
	}

	LIB bool_vector_type FUNC(greaterThanEqual)(vector_arg_type x, vector_arg_type y)
	{
		return bool_vector_type((x.data[Ns] >= y.data[Ns])...);
	}

	LIB bool_vector_type FUNC(equal)(vector_arg_type x, vector_arg_type y)
	{
		return bool_vector_type((x.data[Ns] == y.data[Ns])...);
	}

	LIB bool_vector_type FUNC(notEqual)(vector_arg_type x, vector_arg_type y)
	{
		return bool_vector_type((x.data[Ns] != y.data[Ns])...);
	}

	LIB bool FUNC(any)(typename std::conditional<std::is_same<scalar_type, bool>::value, vector_arg_type, nothing>::type b)
	{
		return (... || b.data[Ns]);
	}

	LIB  bool FUNC(all)(typename std::conditional<std::is_same<scalar_type, bool>::value, vector_arg_type, nothing>::type b)
	{
		return (... && b.data[Ns]);
	}

	LIB bool_vector_type FUNC(_not)(typename std::conditional<std::is_same<scalar_type, bool>::value, vector_arg_type, nothing>::type b)
	{
		return bool_vector_type((!b.data[Ns])...);
	}

//
// 8.13.1 Derivative Functions
// NOTE: can only fake these
	LIB vector_type FUNC(dFdx)(vector_arg_type p)
	{
		return p * scalar_type(.01);
	}

	LIB vector_type FUNC(dFdy)(vector_arg_type p)
	{
		return p * scalar_type(.01);
	}

	LIB vector_type FUNC(fwidth)(vector_arg_type p)
	{
		return FUNC(abs)(FUNC(dFdx)(p)) + FUNC(abs)(FUNC(dFdy)(p));
	}

#undef LIB
#undef FUNC
};

} } // namespace vml::detail