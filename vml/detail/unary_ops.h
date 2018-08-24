// must define `self_type`

#define DEF_OP_UNARY_SCALAR(op)				\
	self_type& operator op(scalar_type s) \
	{										\
		((data[Ns] op s), ...);				\
		return *this;						\
	}

DEF_OP_UNARY_SCALAR(+=)
DEF_OP_UNARY_SCALAR(-=)
DEF_OP_UNARY_SCALAR(*=)
DEF_OP_UNARY_SCALAR(/=)

#define DEF_OP_UNARY_VECTOR(op)				\
	self_type& operator op(const self_type &v) \
	{										\
		((data[Ns] op v.data[Ns]), ...);	\
		return *this;						\
	}

DEF_OP_UNARY_VECTOR(+=)
DEF_OP_UNARY_VECTOR(-=)
#ifdef HAS_UNARY_MUL
DEF_OP_UNARY_VECTOR(*=)
#undef HAS_UNARY_MUL
#endif
DEF_OP_UNARY_VECTOR(/=)

#undef DEF_OP_UNARY_SCALAR
#undef DEF_OP_UNARY_VECTOR

self_type operator -() const
{
	return self_type((-data[Ns])...);
}

//TODO: add  ==, !=