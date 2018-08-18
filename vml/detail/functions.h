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
		constexpr auto one = scalar_type(1);
		constexpr auto zero = scalar_type(0);
		auto k = one - eta * eta * (one - N.dot(N, I) * N.dot(N, I));
		if (k < zero) {
			return vector_type();
		} else {
			return eta * I - (eta * N.dot(N, I) + sqrt(k)) * N;
		}
	}

	using bool_vector_type = vector<bool, Ns...>;

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