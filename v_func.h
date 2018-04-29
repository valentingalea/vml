// NOTE: the -> return declaration is preferred 
// because it allows for strong typing
// meaning that the functions/operators 
// template arguments are less
// likely to accept invalid types 
// NOTE: all the binary operators use an
// implementation that will hopefully
// trigger NRVO
#define DEF_OP_BINARY(op, impl_op, a_type, b_type)	\
	template<class V>								\
	inline auto operator op(a_type a, b_type b) -> typename V::vector_type \
	{												\
		V out = a;									\
		out impl_op b;								\
		return out;									\
	}

DEF_OP_BINARY(+, +=, const V&, const V&)
DEF_OP_BINARY(-, -=, const V&, const V&)
DEF_OP_BINARY(*, *=, const V&, const V&)
DEF_OP_BINARY(/ , /=, const V&, const V&)
DEF_OP_BINARY(+, +=, const V&, typename V::scalar_type)
DEF_OP_BINARY(+, +=, typename V::scalar_type, const V&)
DEF_OP_BINARY(-, -=, const V&, typename V::scalar_type)
DEF_OP_BINARY(-, -=, typename V::scalar_type, const V&)
DEF_OP_BINARY(*, *=, const V&, typename V::scalar_type)
DEF_OP_BINARY(*, *=, typename V::scalar_type, const V&)
DEF_OP_BINARY(/ , /=, const V&, typename V::scalar_type)
DEF_OP_BINARY(/ , /=, typename V::scalar_type, const V&)

//
// Vector utility functions
//
#include <cmath>

template<class V>
inline auto length(const V &v) -> typename V::scalar_type //decltype (v[0])
{
	return std::sqrt(dot(v, v));
}

template<class V>
inline auto normalize(const V &v) -> typename V::vector_type
{
	V out = v;
	out /= length(v);
	return out;
}

template<class V>
inline auto dot(const V &a, const V &b) -> typename V::scalar_type
{
	typename V::scalar_type sum = 0;
	V::iterate([&](int i) {
		sum += a[i] * b[i];
	});
	return sum;
}

template<class V>
inline auto cross(const V &a, const V &b) -> typename V::vector_type
{
	static_assert(V::num_components == 3,
		"cross product only works for vec3");

	return typename V::vector_type(
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	);
}
