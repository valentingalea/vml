#pragma once

#include "vector.h"

namespace swizzle {

template<
	template<typename, size_t> class vector_type,
	typename scalar_type,
	size_t N, size_t M // column order
>
struct matrix
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

	column_type& operator[](size_t i) // row access
	{
		return data[i];
	}

	const column_type& operator[](size_t i) const // row access
	{
		return data[i];
	}

private:
	column_type data[M];
};

} // namespace swizzle