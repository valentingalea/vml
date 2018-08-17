#pragma once

#include <algorithm>

#include "detail/static_for.h"
#include "detail/swizzler.h"
#include "detail/functions.h"
#include "detail/binary_ops.h"

#include "vector_base.h"

namespace swizzle {

template<typename T, size_t... Ns>
struct vector;

namespace util {

template<typename T, size_t... Ns>
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
		using type = detail::swizzler<vector<T, Ns...>, T, sizeof...(indices), indices...>;
	};

	template<size_t x>
	struct swizzler_wrapper_factory<x>
	{
		using type = T; // one component vectors are just scalars
	};

	using base_type = vector_base<T, sizeof...(Ns), swizzler_wrapper_factory>;
};

template <class T>
auto decay(T&& t) -> decltype(t.decay())
{
	return t.decay();
}

template <class T>
typename std::enable_if<
	std::is_scalar<typename std::remove_reference<T>::type >::value,
	T>::type
decay(T&& t)
{
	return t;
}

} // namespace util

template<typename T, size_t... Ns> struct
#ifdef _MSC_VER
__declspec(empty_bases) // https://blogs.msdn.microsoft.com/vcblog/2016/03/30/optimizing-the-layout-of-empty-base-classes-in-vs2015-update-2-3/
#endif
vector :
	public util::vector_base_selector<T, Ns...>::base_type,
	public detail::builtin_func_lib<vector<T, Ns...>, T, Ns...>
//	public detail::binary_vec_ops<vector<T, N>, T, N>
{
	using scalar_type = T;
	using vector_type = vector<T, Ns...>;
	using base_type = typename util::vector_base_selector<T, Ns...>::base_type;
	using decay_type = vector_type;

	// bring in scope the union member
	using base_type::data;

	vector()
	{
		((data[Ns] = 0), ...);
	}

	vector(scalar_type s)
	{
		((data[Ns] = s), ...);
	}

	template<class Func>
	static constexpr void iterate(Func &&f)
	{
		detail::static_for<0, sizeof...(Ns)>()(std::forward<Func>(f));
	}

	template<typename... S>
	explicit vector(S&&... args)
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

#include "detail/unary_ops.h"

private:
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