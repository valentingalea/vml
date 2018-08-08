#pragma once

// stupid fix for C++17 on c4droid
#define throw(...)

#include <algorithm>

#include "detail/static_for.h"
#include "detail/swizzler.h"
#include "vector_base.h"

namespace swizzle {

template<typename T, size_t N>
struct vector;

namespace util {

template<typename T, size_t N>
struct vector_base_selector
{
	template<size_t... indices> // 0 for x (or r), 1 for y (or g), etc 
	struct swizzler_wrapper_factory
	{
		// NOTE: need to pass the equivalent vector type
		// this can be different than the current 'host' vector
		// for ex:
		// .xy is vec2 that is part of a vec3
		// .xy is also vec2 but part of a vec4
		// they need to be same underlying type
		using type = detail::swizzler<vector<T, sizeof...(indices)>, T, N, indices...>;
	};

	template<size_t x>
	struct swizzler_wrapper_factory<x>
	{
		using type = T;
	};

	using base_type = vector_base<T, N, swizzler_wrapper_factory>;
};

} // namespace util

template<typename T, size_t N>
struct vector
	: public util::vector_base_selector<T, N>::base_type
{
	using scalar_type = T;
	using vector_type = vector<T, N>;
	using base_type = typename util::vector_base_selector<T, N>::base_type;

	// bring in scope the union member
	using base_type::data;

	vector()
	{
		iterate([this](size_t i) {
			data[i] = 0;
		});
	}

	vector(scalar_type s)
	{
		iterate([s, this](size_t i) {
			data[i] = s;
		});
	}

	template<typename... S>
	explicit vector(S... args)
	{
		static_assert(
			(sizeof...(args) <= N),
			"mismatch number of vector init arguments");

		// dummy forwarding structure
		struct constructor
		{
			inline constructor(...) {}
		};

		size_t i = 0;
		constructor(
			// - the use of {} init list guarantees left to right
			// processing order
			// - the ... will basically expand and paste in 
			// each function argument
			// - which in turn we feed to a special function
			// that overloads for every vector element type
			{ construct_at_index(i, std::forward<S>(args)) ... }
		);
	}

	scalar_type const operator[](size_t i) const
	{
		return data[i];
	}

	scalar_type& operator[](size_t i)
	{
		return data[i];
	}

private:
	template<class Func>
	constexpr void iterate(Func &&f)
	{
		detail::static_for<0, N>()(std::forward<Func>(f));
	}

	bool construct_at_index(size_t &i, scalar_type arg)
	{
		data[i++] = arg;
		return true; // dummy return
					 // just because it wil be called in a {} init list
	}

	template<typename Other, size_t Other_N>
	bool construct_at_index(size_t &i, vector<Other, Other_N> &&arg)
	{
		constexpr auto count = std::min(N, Other_N);
		detail::static_for<0, count>()([&](size_t j) {
			data[i++] = arg.data[j];
		});
		return true;
	}
};

} // namespace swizzle