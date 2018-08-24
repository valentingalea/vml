#pragma once

#include "vector.h"

namespace vml {

template<
	typename scalar_type,
	template<typename, size_t...> class vector_type,
	size_t N, size_t M
>
struct matrix
	//TODO: binary operators
{
	using column_type = typename detail::vec_equiv<scalar_type, N>::type;
	using row_type = typename detail::vec_equiv<scalar_type, M>::type;

	matrix() = default; // zeroes all data

	explicit matrix(scalar_type s) // fill in diagonally
	{
		constexpr auto num = N < M ? N : M;
		detail::static_for<0, num>()([&](size_t i) {
			data[i][i] = s;
		});
	}

	template<typename... Args,
		class = typename std::enable_if<
			(sizeof... (Args) >= 2)
		>::type>
	explicit matrix(Args&&... args)
	{
		static_assert((sizeof...(args) <= N*M), "too many arguments");

		size_t i = 0; //TODO: get rid of this
		(construct_at_index(i, detail::decay(std::forward<Args>(args))), ...);
	}

	//TODO: constructor from smaller matrices 

	column_type& operator[](size_t i) // row access
	{
		return data[i];
	}

	const column_type& operator[](size_t i) const // row access
	{
		return data[i];
	}

	//TODO: unary operators

private:
// these constructors advance down along columns
// continuing on the right neighbour on overspill
// granted ofc they don't exceed overal N * M
	void construct_at_index(size_t &i, scalar_type arg)
	{
		data[i / N][i % N] = arg;
		i++;
	}

	template<typename Other, size_t... Other_Ns>
	void construct_at_index(size_t &i, vector_type<Other, Other_Ns...> &&arg)
	{
		//TODO: do not go over N*M
		detail::static_for<0, sizeof...(Other_Ns)>()([&](size_t j) {
			data[i / N][i % N] = arg[j];
			i++;
		});
	}

	column_type data[M];
};

} // namespace vml