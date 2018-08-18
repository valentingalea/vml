#define DEF_OP_UNARY_SCALAR(op)				\
	vector_type& operator op(scalar_type s) \
	{										\
		((data[Ns] op s), ...);				\
		return *this;						\
	}

DEF_OP_UNARY_SCALAR(+= )
DEF_OP_UNARY_SCALAR(-= )
DEF_OP_UNARY_SCALAR(*= )
DEF_OP_UNARY_SCALAR(/= )

#define DEF_OP_UNARY_VECTOR(op)				\
	vector_type& operator op(const vector_type &v) \
	{										\
		((data[Ns] op v.data[Ns]), ...);	\
		return *this;						\
	}

DEF_OP_UNARY_VECTOR(+=)
DEF_OP_UNARY_VECTOR(-=)
DEF_OP_UNARY_VECTOR(*=)
DEF_OP_UNARY_VECTOR(/=)

#undef DEF_OP_UNARY_SCALAR
#undef DEF_OP_UNARY_VECTOR

vector_type operator -() const
{
	return vector_type((-data[Ns])...);
}

//TODO: add  ==, !=