#pragma once

#include <algorithm>

#include "detail/util.h"
#include "detail/swizzler.h"
#include "detail/functions.h"
#include "detail/binary_ops.h"

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

} // namespace util

template<typename T, size_t N> struct
#ifdef _MSC_VER
__declspec(empty_bases) // https://blogs.msdn.microsoft.com/vcblog/2016/03/30/optimizing-the-layout-of-empty-base-classes-in-vs2015-update-2-3/
#endif
vector :
	public util::vector_base_selector<T, N>::base_type,
	public detail::builtin_func_lib<vector<T, N>, T, N>,
	public detail::binary_vec_ops<vector<T, N>, T, N>
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

	explicit vector(scalar_type s)
	{
		iterate([s, this](size_t i) {
			data[i] = s;
		});
	}

	vector(const vector_type &) = default;
	vector(vector_type &&) = default;

	template<typename A0, typename... Args,
		class = typename std::enable_if<
			((sizeof... (Args) >= 1) ||
			((sizeof... (Args) == 0) && !std::is_scalar_v<A0>))
		>::type>
	explicit vector(A0&& a0, Args&&... args)
	{
		static_assert((sizeof...(args) < N), "too many arguments");

		size_t i = 0; //TODO: get rid of this and introduce template get_size

		// consume the first one
		construct_at_index(i, detail::decay(std::forward<A0>(a0)));

		// consume the rest, if any
		(construct_at_index(i, detail::decay(std::forward<Args>(args))), ...);
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

	template<class Func>
	static constexpr void iterate(Func &&f)
	{
		detail::static_for<0, N>()(std::forward<Func>(f));
	}

#include "detail/unary_ops.h"

	//TODO: add matrix multiply

private:
	void construct_at_index(size_t &i, scalar_type arg)
	{
		data[i++] = arg;
	}

	template<typename Other, size_t Other_N>
	void construct_at_index(size_t &i, const vector<Other, Other_N> &arg)
	{
		constexpr auto count = std::min(N, Other_N);
		detail::static_for<0, count>()([&](size_t j) {
			data[i++] = arg.data[j];
		});
	}
};

} // namespace swizzle