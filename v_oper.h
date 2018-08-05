#define DEF_OP_UNARY_SCALAR(op)				\
	vector_type& operator op(scalar_type s) \
	{										\
		iterate([s, this](size_t i){			\
			data[i] op s;					\
		});									\
		return *this;						\
	}

DEF_OP_UNARY_SCALAR(+=)
DEF_OP_UNARY_SCALAR(-=)
DEF_OP_UNARY_SCALAR(*=)
DEF_OP_UNARY_SCALAR(/=)

#define DEF_OP_UNARY_VECTOR(op)				\
	vector_type& operator op(const vector_type &v) \
	{										\
		iterate([&](size_t i){					\
			data[i] op v[i];				\
		});									\
		return *this;						\
	}

DEF_OP_UNARY_VECTOR(+=)
DEF_OP_UNARY_VECTOR(-=)
DEF_OP_UNARY_VECTOR(*=)
DEF_OP_UNARY_VECTOR(/=)
