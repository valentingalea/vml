#pragma once

#include "vector.h"

namespace vml {

template<size_t...>
struct indices_pack;

template<
	typename,
	template<typename, size_t...> class vector_type,
	typename...
>
struct matrix;

template<
	typename scalar_type,
	template<typename, size_t...> class vector_type,
	size_t... Columns,
	size_t... Rows
>
struct matrix<scalar_type, vector_type, indices_pack<Columns...>, indices_pack<Rows...>>
	: public detail::binary_vec_ops<matrix<scalar_type, vector_type, indices_pack<Columns...>, indices_pack<Rows...>>, scalar_type>
{
	static constexpr auto N = sizeof...(Columns);
	static constexpr auto M = sizeof...(Rows);
	using column_type = vector_type<scalar_type, Columns...>;
	using row_type = vector_type<scalar_type, Rows...>;

	matrix() = default; // zeroes all data

	template<typename S, class = typename std::enable_if<
		std::is_same<S, scalar_type>::value && (N == M)>::type>
	explicit matrix(S s) // fill in diagonally
	{
		((data[Rows][Rows] = s), ...);
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

	using self_type = matrix;
	using other_type = self_type;
#define Is Rows
#include "detail/unary_ops.h"

	friend column_type operator *(const matrix &m, const row_type &v)
	{
		return mul(m, v);
	}

	friend row_type operator *(const column_type &v, const matrix &m)
	{
		return mul(v, m);
	}

	matrix& operator *=(const matrix &m)
	{
		return *this = mul(*this, m);
	}

	static column_type mul(const matrix &m, const row_type &v)
	{
		column_type out;
		((out[Rows] = v.lib_dot(v, m.row(Rows))), ...);
		return out;
	}

	static row_type mul(const column_type &v, const matrix &m)
	{
		row_type out;
		((out[Columns] = v.lib_dot(v, m.column(Columns))), ...);
		return out;
	}

	template<size_t... OtherRows>
	static auto mul(
		const matrix &m1,
		const matrix<scalar_type, vector_type, indices_pack<Columns...>, indices_pack<OtherRows...>> &m2)
	{
		matrix<scalar_type, vector_type, indices_pack<Columns...>, indices_pack<OtherRows...>> out;
		((out[Columns] = m1 * m2.column(Columns)), ...);
		return out;
	}

	const column_type& column(size_t i) const
	{
		return data[i];
	}

	row_type row(size_t i) const
	{
		row_type out;
		((out[Rows] = data[Rows][i]), ...);

		return out;
	}

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