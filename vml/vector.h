#pragma once

#include <type_traits>
#include <utility>

#include "detail/util.h"
#include "detail/swizzler.h"
#include "detail/functions.h"
#include "detail/binary_ops.h"
#include "detail/vector_base.h"

namespace vml {

template<typename T, size_t... Ns>
struct _MSC_FIX_EBO vector :
	public detail::vector_base_selector<T, Ns...>::base_type,
	public detail::builtin_func_lib<vector, T, Ns...>,
	public std::conditional<sizeof...(Ns) != 1, // no binary ops for promoted scalar
		detail::binary_vec_ops<vector<T, Ns...>, T>, detail::nothing>::type
{
	static constexpr auto num_components = sizeof...(Ns);

	using scalar_type = T;
	using vector_type = vector<T, Ns...>;
	using base_type = typename detail::vector_base_selector<T, Ns...>::base_type;
	using decay_type = typename std::conditional<num_components == 1, scalar_type, vector_type>::type;

	// bring in scope the union member
	using base_type::data;

	vector()
	{
		((data[Ns] = 0), ...);
	}

	vector(typename std::conditional<num_components == 1, scalar_type, detail::nothing>::type s)
	{
		data[0] = s;
	}

	explicit vector(typename std::conditional<num_components != 1, scalar_type, detail::nothing>::type s)
	{
		((data[Ns] = s), ...);
	}

	template<typename A0, typename... Args,
		class = typename std::enable_if<
			((sizeof... (Args) >= 1) ||
			((sizeof... (Args) == 0) && !std::is_scalar<A0>::value))
		>::type>
	explicit vector(A0&& a0, Args&&... args)
	{
		static_assert((sizeof...(args) < num_components), "too many arguments");

#define CTOR_FOLD
#ifdef CTOR_FOLD
		size_t i = 0;

		 construct_at_index(i, detail::decay(std::forward<A0>(a0)));
		(construct_at_index(i, detail::decay(std::forward<Args>(args))), ...);
#else
		static_recurse<0>(std::forward<A0>(a0), std::forward<Args>(args)...);
#endif
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

	operator typename std::conditional<num_components == 1, scalar_type, detail::nothing>::type() const
	{
		return data[0];
	}

	using self_type = vector_type;
	using other_type = self_type;
#define Is Ns
#define HAS_UNARY_MUL
#include "detail/unary_ops.h"

	//TODO: add matrix multiply

private:
#ifdef CTOR_FOLD
	void construct_at_index(size_t &i, scalar_type arg)
	{
		data[i++] = arg;
	}

	template<typename Other, size_t... Other_Ns>
	void construct_at_index(size_t &i, const vector<Other, Other_Ns...> &arg)
	{
		constexpr auto other_num = vector<Other, Other_Ns...>::num_components;
		constexpr auto count = num_components <= other_num ? num_components : other_num;

		detail::static_for<0, count>()([&](size_t j) {
			data[i++] = arg.data[j];
		});
	}
#else
	template<size_t i>
	void construct_at_index(scalar_type arg)
	{
		data[i] = arg;
	}

	template<size_t i, typename Other, size_t... Other_Ns>
	void construct_at_index(const vector<Other, Other_Ns...> &arg)
	{
		constexpr auto other_num = vector<Other, Other_Ns...>::num_components;
		constexpr auto count = (i + other_num) > num_components ? num_components : (i + other_num);
		detail::static_for<i, count>()([&](size_t j) {
			data[j] = arg.data[j - i];
		});
	}

	template<size_t I, typename Arg0, typename... Args>
	void static_recurse(Arg0&& a0, Args&&... args)
	{
		construct_at_index<I>(detail::decay(std::forward<Arg0>(a0)));
		static_recurse<I + detail::get_size<Arg0>()>(std::forward<Args>(args)...);
	}

	template<size_t I>
	void static_recurse()
	{}
#endif
};

} // namespace vml

// needs to be after everything has been defined
#include "detail/traits.h"
