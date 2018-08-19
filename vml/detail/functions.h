#pragma once

#include <cmath>

namespace vml { namespace detail
{

template<template<typename, size_t...> class vector, typename scalar_type, size_t... Ns>
struct builtin_func_lib
{
#ifdef MSVC_VER
#define INL __forceinline
#else
#define INL __attribute__((always_inline))
#endif

	using vector_type = vector<scalar_type, Ns...>;
	using vector_arg_type = const vector_type &;
	using bool_vector_type = vector<bool, Ns...>;

	static constexpr auto one = scalar_type(1);
	static constexpr auto zero = scalar_type(0);

//
// 8.1 Angle and Trigonometry Function
//
	friend vector_type radians(vector_arg_type degrees)
	{
		constexpr auto pi_over_180 = scalar_type(3.14159265358979323846 / 180);
		return vector_type((degrees.data[Ns] * pi_over_180)...);
	}

	friend vector_type degrees(vector_arg_type radians)
	{
		constexpr auto _180_over_pi = scalar_type(180 / 3.14159265358979323846);
		return vector_type((radians.data[Ns] * _180_over_pi)...);
	}

	friend vector_type sin(vector_arg_type t)
	{
		using namespace std;
		return vector_type(sin(t.data[Ns])...);
	}

	friend vector_type cos(vector_arg_type t)
	{
		using namespace std;
		return vector_type(cos(t.data[Ns])...);
	}

	friend vector_type tan(vector_arg_type t)
	{
		using namespace std;
		return vector_type(tan(t.data[Ns])...);
	}

	friend vector_type asin(vector_arg_type t)
	{
		using namespace std;
		return vector_type(asin(t.data[Ns])...);
	}

	friend vector_type acos(vector_arg_type t)
	{
		using namespace std;
		return vector_type(acos(t.data[Ns])...);
	}

	friend vector_type atan(vector_arg_type y, vector_arg_type x)
	{
		using namespace std;
		return vector_type(atan2(y.data[Ns] / x.data[Ns])...);
	}

	friend vector_type atan(vector_arg_type t)
	{
		using namespace std;
		return vector_type(atan(t.data[Ns])...);
	}
	// sinh cosh tanh
	// asinh acosh atanh

//
// 8.2 Exponential Functions
//
	//TODO: for now <cmath> stuff should be enough for most purposes
	// pow exp log
	// exp2 log2
	// sqrt inversesqrt

//
// 8.3 Common Functions
//
	//TODO:
	// abs sign
	// floor trunc round roundEven ceil fract mod modf
	
	friend vector_type min(vector_arg_type left, vector_arg_type right)
	{
		return vector_type((left.data[Ns] < right.data[Ns] ? left.data[Ns] : right.data[Ns])...);
	}

	friend vector_type min(vector_arg_type left, scalar_type right)
	{
		return vector_type((left.data[Ns] < right ? left.data[Ns] : right)...);
	}

	friend vector_type max(vector_arg_type left, vector_arg_type right)
	{
		return vector_type((left.data[Ns] > right.data[Ns] ? left.data[Ns] : right.data[Ns])...);
	}

	friend vector_type max(vector_arg_type left, scalar_type right)
	{
		return vector_type((left.data[Ns] > right ? left.data[Ns] : right)...);
	}

	friend vector_type clamp(vector_arg_type x, vector_arg_type minVal, vector_arg_type maxVal)
	{
		return min(max(x, minVal), maxVal);
	}

	friend vector_type clamp(vector_arg_type x, scalar_type minVal, scalar_type maxVal)
	{
		return min(max(x, minVal), maxVal);
	}

	friend vector_type mix(vector_arg_type x, vector_arg_type y, vector_arg_type a)
	{
		return x * (vector_type(scalar_type(1)) - a) + y * a;
	}

	friend vector_type mix(vector_arg_type x, vector_arg_type y, scalar_type a)
	{
		return x * (scalar_type(1) - a) + y * a;
	}

	//TODO: Doesn't work
	//friend vector_type mix(vector_arg_type x, vector_arg_type y, const bool_vector_type &a)
	//{
	//	return vector_type((a.data[Ns] ? y.data[Ns] : x.data[Ns])...);
	//}

	friend vector_type step(scalar_type edge, vector_arg_type x)
	{
		return vector_type((x.data[Ns] < edge ? scalar_type(0) : scalar_type(1))...);
	}

	friend vector_type step(vector_arg_type edge, vector_arg_type x)
	{
		return vector_type((x.data[Ns] < edge.data[Ns] ? scalar_type(0) : scalar_type(1))...);
	}

	friend vector_type smoothstep(scalar_type edge0, scalar_type edge1, vector_arg_type x)
	{
		auto t = clamp((x - edge0) / (edge1 - edge0), zero, one);
		return t * t * (scalar_type(3) - scalar_type(2) * t);
	}

	friend vector_type smoothstep(vector_arg_type edge0, vector_arg_type edge1, vector_arg_type x)
	{
		auto t = clamp((x - edge0) / (edge1 - edge0), zero, one);
		return t * t * (scalar_type(3) - scalar_type(2) * t);
	}
//
// 8.5 Geometric functions
//
	friend scalar_type length(vector_arg_type v)
	{
		return std::sqrt(v.dot(v, v));
	}

	friend scalar_type distance(vector_arg_type p0, const vector_arg_type p1)
	{
		return length(p0 - p1);
	}

	friend vector_type normalize(vector_arg_type v)
	{
		vector_type out = v;
		out /= length(v);
		return out;
	}

	scalar_type dot(vector_arg_type a, vector_arg_type b) const // needs to be member otherwise fold expression doesn't work
	{
		scalar_type sum = 0;
		((sum += a.data[Ns] * b.data[Ns]), ...);
		return sum;
	}

	friend scalar_type dot(vector_arg_type a, vector_arg_type b)
	{
		return a.dot(a, b);
	}

	friend vector_type cross(vector_arg_type a, vector_arg_type b)
	{
		static_assert(vector_type::num_components == 3, "cross product only works for vec3");

		return vector_type(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
		);
	}

	friend vector_type faceforward(vector_arg_type N, vector_arg_type I, vector_arg_type Nref)
	{
		return (N.dot(Nref, I) < scalar_type(0) ? N : (-N));
	}

	friend vector_type reflect(vector_arg_type I, vector_arg_type N)
	{
		return (I - scalar_type(2) * N.dot(I, N) * N);
	}

	friend vector_type refract(vector_arg_type I, vector_arg_type N, scalar_type eta)
	{
		auto k = one - eta * eta * (one - N.dot(N, I) * N.dot(N, I));
		if (k < zero) {
			return vector_type();
		} else {
			return eta * I - (eta * N.dot(N, I) + sqrt(k)) * N;
		}
	}

//
// 8.7 Vector Relational Functions
//
	friend bool_vector_type lessThan(vector_arg_type x, vector_arg_type y)
	{
		return bool_vector_type((x.data[Ns] < y.data[Ns])...);
	}

	friend bool_vector_type lessThanEqual(vector_arg_type x, vector_arg_type y)
	{
		return bool_vector_type((x.data[Ns] <= y.data[Ns])...);
	}

	friend bool_vector_type greaterThan(vector_arg_type x, vector_arg_type y)
	{
		return bool_vector_type((x.data[Ns] > y.data[Ns])...);
	}

	friend bool_vector_type greaterThanEqual(vector_arg_type x, vector_arg_type y)
	{
		return bool_vector_type((x.data[Ns] >= y.data[Ns])...);
	}

	friend bool_vector_type equal(vector_arg_type x, vector_arg_type y)
	{
		return bool_vector_type((x.data[Ns] == y.data[Ns])...);
	}

	friend bool_vector_type notEqual(vector_arg_type x, vector_arg_type y)
	{
		return bool_vector_type((x.data[Ns] != y.data[Ns])...);
	}
#if 0 //TODO: MSVC bug
	bool any(const bool_vector_type &b) const
	{
		return (... || b.data[Ns]);
	}
	friend bool any(typename std::conditional<std::is_same<scalar_type, bool>::value, vector_arg_type, std::false_type>::type b)
	{
		return b.any(b); // MSVC: doesn't see the pack expansion if tried here
	}

	bool all(const bool_vector_type &b) const
	{
		return (... && b.data[Ns]);
	}
	friend bool all(typename std::conditional<std::is_same<scalar_type, bool>::value, vector_arg_type, std::false_type>::type b)
	{
		return b.all(b);
	}

	friend bool_vector_type _not(typename std::conditional<std::is_same<scalar_type, bool>::value, vector_arg_type, std::false_type>::type b)
	{
		return bool_vector_type((!b.data[Ns])...);
	}
#endif

#undef INL
};

} } // namespace vml::detail