#pragma once

#include "vector.h"

namespace swizzle {

template<
	template<typename, size_t> class vector_type,
	typename scalar_type,
	size_t N, size_t M // column order
>
struct matrix
	//TODO: binary operators
{
	using column_type = vector_type<scalar_type, N>;
	using row_type = vector_type<scalar_type, M>;

	matrix() = default; // zeroes all data

	matrix(scalar_type s) // fill in diagonally
	{
		detail::static_for<0, std::min(N, M)>()([&](size_t i) {
			data[i][i] = s;
		});
	}

	template<typename... S>
	explicit matrix(S... args)
	{
		static_assert((sizeof...(args) <= N*M), "too many arguments");

		size_t i = 0; //TODO: get rid of this
		(construct_at_index(i, detail::decay(std::forward<S>(args))), ...);
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

	template<typename Other, size_t Other_N>
	void construct_at_index(size_t &i, vector<Other, Other_N> &&arg)
	{
		//TODO: do not go over N*M
		detail::static_for<0, Other_N>()([&](size_t j) {
			data[i / N][i % N] = arg[j];
			i++;
		});
	}

	column_type data[M];
};

} // namespace swizzle