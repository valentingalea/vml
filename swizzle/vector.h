#pragma once

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
		using type = T; // one component vectors are just scalars
	};

	using base_type = vector_base<T, N, swizzler_wrapper_factory>;
};

template <class T>
auto decay(T&& t) -> decltype(t.decay())
{
	return t.decay();
}

template <class T>
typename std::enable_if_t<std::is_scalar_v<typename std::remove_reference_t<T>>, T> decay(T&& t)
{
	return t;
}

} // namespace util

template<typename T, size_t N>
struct vector
	: public util::vector_base_selector<T, N>::base_type
{
	using scalar_type = T;
	using vector_type = vector<T, N>;
	using base_type = typename util::vector_base_selector<T, N>::base_type;
	using decay_type = typename std::conditional_t<N == 1, scalar_type, vector>;

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
		static_assert((sizeof...(args) <= N), "mismatch number of vector init arguments");

		size_t i = 0;
		(construct_at_index(i, util::decay(std::forward<S>(args))), ...);
	}

	scalar_type const operator[](size_t i) const
	{
		return data[i];
	}

	scalar_type& operator[](size_t i)
	{
		return data[i];
	}

	decay_type decay() const
	{
		return static_cast<const decay_type&>(*this);
	}

private:
	template<class Func>
	constexpr void iterate(Func &&f)
	{
		detail::static_for<0, N>()(std::forward<Func>(f));
	}

	void construct_at_index(size_t &i, scalar_type arg)
	{
		data[i++] = arg;
	}

	template<typename Other, size_t Other_N>
	void construct_at_index(size_t &i, vector<Other, Other_N> &&arg)
	{
		constexpr auto count = std::min(N, Other_N);
		detail::static_for<0, count>()([&](size_t j) {
			data[i++] = arg.data[j];
		});
	}
};

} // namespace swizzle