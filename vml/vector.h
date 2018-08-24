#pragma once

#include <type_traits>

#include "detail/swizzler.h"
#include "detail/functions.h"
#include "detail/binary_ops.h"
#include "detail/util.h"

#include "vector_base.h"

namespace vml {

template<typename T, size_t... Ns> struct
#ifdef _MSC_VER
__declspec(empty_bases) // https://blogs.msdn.microsoft.com/vcblog/2016/03/30/optimizing-the-layout-of-empty-base-classes-in-vs2015-update-2-3/
#endif
vector :
	public detail::vector_base_selector<T, Ns...>::base_type,
	public detail::builtin_func_lib<vector, T, Ns...>,
	public detail::binary_vec_ops<vector<T, Ns...>, T>
{
	static constexpr auto num_components = sizeof...(Ns);

	using scalar_type = T;
	using vector_type = vector<T, Ns...>;
	using base_type = typename detail::vector_base_selector<T, Ns...>::base_type;
	using decay_type = vector_type; // typename util::vec_equiv<T, num_components>::type;

	// bring in scope the union member
	using base_type::data;

	vector()
	{
		((data[Ns] = 0), ...);
	}

	/*explicit*/ vector(scalar_type s)
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
		static_assert((sizeof...(args) <= num_components), "too many arguments");

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
		constexpr auto count = num_components <= vector<Other, Other_Ns...>::num_components ?
			num_components : vector<Other, Other_Ns...>::num_components;
		detail::static_for<0, count>()([&](size_t j) {
			data[i++] = arg.data[j];
		});
	}
};

} // namespace vml