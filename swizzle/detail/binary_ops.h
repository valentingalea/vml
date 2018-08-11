#pragma once

namespace swizzle { namespace detail
{

template<typename vector_type, typename scalar_type, size_t N>
struct binary_vec_ops
{
#define DEF_OP_BINARY(op, impl_op, a_type, b_type)	\
	friend vector_type operator op(a_type a, b_type b) \
	{												\
		vector_type out = a;						\
		out impl_op b;								\
		return out;									\
	}

	DEF_OP_BINARY(+, +=, const vector_type &, const vector_type &)
	DEF_OP_BINARY(-, -=, const vector_type &, const vector_type &)
	DEF_OP_BINARY(*, *=, const vector_type &, const vector_type &)
	DEF_OP_BINARY(/ , /=, const vector_type &, const vector_type &)
	DEF_OP_BINARY(+, +=, const vector_type &, scalar_type)
	DEF_OP_BINARY(+, +=, scalar_type, const vector_type &)
	DEF_OP_BINARY(-, -=, const vector_type &, scalar_type)
	DEF_OP_BINARY(-, -=, scalar_type, const vector_type &)
	DEF_OP_BINARY(*, *=, const vector_type &, scalar_type)
	DEF_OP_BINARY(*, *=, scalar_type, const vector_type &)
	DEF_OP_BINARY(/ , /=, const vector_type &, scalar_type)
	DEF_OP_BINARY(/ , /=, scalar_type, const vector_type &)

#undef DEF_OP_BINARY
};

} } // namespace swizzle::detail