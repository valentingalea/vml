// must define `self_type` and `other_type`
// must define `Is` as the param pack expansion of the indices

#define DEF_OP_UNARY_SCALAR(op)				\
	self_type& operator op(scalar_type s) \
	{										\
		((data[Is] op s), ...);				\
		return *this;						\
	}

DEF_OP_UNARY_SCALAR(+=)
DEF_OP_UNARY_SCALAR(-=)
DEF_OP_UNARY_SCALAR(*=)
DEF_OP_UNARY_SCALAR(/=)

#define DEF_OP_UNARY_VECTOR(op)				\
	self_type& operator op(const other_type &v) \
	{										\
		((data[Is] op v.data[Is]), ...);	\
		return *this;						\
	}

DEF_OP_UNARY_VECTOR(+=)
DEF_OP_UNARY_VECTOR(-=)
#ifdef HAS_UNARY_MUL
DEF_OP_UNARY_VECTOR(*=)
#undef HAS_UNARY_MUL
#endif
DEF_OP_UNARY_VECTOR(/=)

#ifndef OMIT_NEG_OP
self_type operator -() const
{
	return self_type((-data[Is])...);
}
#endif // !OMIT_NEG_OP

//TODO: add  ==, !=

#undef DEF_OP_UNARY_SCALAR
#undef DEF_OP_UNARY_VECTOR
#undef Is