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
#if 1
		static_assert((sizeof...(args) <= num_components), "too many arguments");

		size_t i = 0; //TODO: get rid of this and introduce template get_size

					  // consume the first one
		construct_at_index(i, detail::decay(std::forward<A0>(a0)));

		// consume the rest, if any
		(construct_at_index(i, detail::decay(std::forward<Args>(args))), ...);
#else
		iterate_construct<0, num_components>(std::forward<A0>(a0), std::forward<Args>(args)...);
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
#define Is Ns
#define HAS_UNARY_MUL
#include "detail/unary_ops.h"

	//TODO: add matrix multiply

private:
	template<size_t I, size_t IMax, typename Arg0, typename... Args>
	void iterate_construct(Arg0&& a0, Args&&... args)
	{
		if constexpr (I < IMax) {
			size_t i = I;
			construct_at_index(i, detail::decay(std::forward<Arg0>(a0)));
			iterate_construct<I + detail::get_size<Arg0>(), IMax>(std::forward<Args>(args)...);
		}
	}

	template<size_t I, size_t IMax>
	void iterate_construct()
	{}

	void construct_at_index(size_t &i, scalar_type arg)
	{
		data[i++] = arg;
	}

	template<typename Other, size_t... Other_Ns>
	void construct_at_index(size_t &i, const vector<Other, Other_Ns...> &arg)
	{
		constexpr auto count = num_components <= vector<Other, Other_Ns...>::num_components ?
			num_components : vector<Other, Other_Ns...>::num_components;
		detail::static_for<0, count>()([&](size_t j) {
			data[i++] = arg.data[j];
		});
	}
};

} // namespace vml

// needs to be after everything has been defined
#include "detail/traits.h"
