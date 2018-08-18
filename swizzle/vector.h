#pragma once

#include <algorithm>

#include "detail/util.h"
#include "detail/swizzler.h"
#include "detail/functions.h"
#include "detail/binary_ops.h"

#include "vector_base.h"

namespace vml {

template<typename T, size_t... Ns>
struct vector;

namespace util {

template<typename, size_t>
struct vec_equiv;

template<typename T>
struct vec_equiv<T, 2>
{
	using type = vector<T, 0, 1>;
};

template<typename T>
struct vec_equiv<T, 3>
{
	using type = vector<T, 0, 1, 2>;
};

template<typename T>
struct vec_equiv<T, 4>
{
	using type = vector<T, 0, 1, 2, 3>;
};

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
		using type = detail::swizzler<
			typename vec_equiv<T, sizeof...(indices)>::type, T, sizeof...(Ns), indices...>;
	};

	template<size_t x>
	struct swizzler_wrapper_factory<x>
	{
		using type = T; // one component vectors are just scalars
	};

	using base_type = vector_base<T, sizeof...(Ns), swizzler_wrapper_factory>;
};

} // namespace util

template<typename T, size_t... Ns> struct
#ifdef _MSC_VER
__declspec(empty_bases) // https://blogs.msdn.microsoft.com/vcblog/2016/03/30/optimizing-the-layout-of-empty-base-classes-in-vs2015-update-2-3/
#endif
vector :
	public util::vector_base_selector<T, Ns...>::base_type,
	public detail::builtin_func_lib<vector<T, Ns...>, T, Ns...>,
	public detail::binary_vec_ops<vector<T, Ns...>, T>
{
	static constexpr auto num_components = sizeof...(Ns);

	using scalar_type = T;
	using vector_type = vector<T, Ns...>;
	using base_type = typename util::vector_base_selector<T, Ns...>::base_type;
	using decay_type = vector_type; // typename util::vec_equiv<T, num_components>::type;

	// bring in scope the union member
	using base_type::data;

	vector()
	{
		((data[Ns] = 0), ...);
	}

	explicit vector(scalar_type s)
	{
		((data[Ns] = s), ...);
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
	//	static_assert((sizeof...(args) < N), "too many arguments");

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

#include "detail/unary_ops.h"

	//TODO: add matrix multiply

private:
	void construct_at_index(size_t &i, scalar_type arg)
	{
		data[i++] = arg;
	}

	template<typename Other, size_t... Other_Ns>
	void construct_at_index(size_t &i, const vector<Other, Other_Ns...> &arg)
	{
		constexpr auto count = std::min(num_components, vector<Other, Other_Ns...>::num_components);
		detail::static_for<0, count>()([&](size_t j) {
			data[i++] = arg.data[j];
		});
	}
};

} // namespace vml