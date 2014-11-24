#include <cstdio>
#include <cmath>
#include <utility>
#include <static_for.h>

template<typename T, int N>
struct vector;

template<typename T, int P, int N>
struct swizzler
{
	typedef vector<T, P> parent_type;
	
	T data[N];
	
	operator parent_type() const
	{
		parent_type v;
		return v;
	}
};

template<typename T, int N>
struct vector_base
{
	union
	{
		T data[N];
	};
};

template<typename T>
struct vector_base<T, 2>
{
	union
	{
		T data[2];
		struct { T x, y; };
		struct { T s, t; };
		struct { T u, v; };
	};
};

template<typename T>
struct vector_base<T, 3>
{
	union
	{
		T data[3];
		struct { T x, y, z; };
		struct { T r, g, b; };
		swizzler<T, 3, 2> xy;
	};
};

template<typename T>
struct vector_base<T, 4>
{
	union
	{
		T data[4];
		struct { T x, y, z, w; };
		struct { T r, g, b, a; };
	};
};

// TODO: swizzle support

template<typename T, int N>
struct vector : public vector_base<T, N>
{
	typedef T scalar_type;
	typedef vector<T, N> vector_type;
	typedef vector_base<T, N> base_type;
	static const int num_components = N;
	using base_type::data;
	
	vector()
	{
		iterate([&](int i){ data[i] = 0; });
	}
	
	vector(scalar_type s)
	{
		iterate([&](int i){ data[i] = s; });
	}
	
	vector(const vector_type &) = default;
	vector_type& operator=(const vector &) = default;
	
	vector(vector_type &&) = default;
	vector_type& operator=(vector &&) = default;
	
	bool elem_set(T *data, int &i, T &&arg)
	{
		data[i++] = arg;
		return true;
	}
	
	bool elem_set(T *data, int &i, vector<T, 2> &&arg)
	{
		data[i++] = arg.x;
		data[i++] = arg.y;
		return true;
	}
	
	bool elem_set(T *data, int &i, vector<T, 3> &&arg)
	{
		data[i++] = arg.x;
		data[i++] = arg.y;
		data[i++] = arg.z;
		return true;
	}
	
	template<typename TT, int PP, int NN>
	bool elem_set(T *data, int &i, swizzler<TT, PP, NN> &&arg)
	{
		return true;
	}

	template<typename... S>
	vector(S... args)
	{
		static_assert(
		  (sizeof...(args) <= num_components),
		  "mismatch number of vector init arguments");

		// dummy structure that is only used
		// to initialise it with an std::initalizer_list
		// where we will get the chance to run
		// special code for each list element 
		struct constructor
		{
			constructor(...) {}
		};
			
		int i = 0;
		constructor(
		// - the use of {} init list guarantees left to right
		// processing order
		// - the ... will basically expand and paste in 
		// each function argument
		// - which in turn we feed to a special function
		// that overloads for every vector element type
		{ elem_set(data, i, std::forward<S>(args)) ... }
		);
	}
	
	template<class Func>
	static void iterate(Func f)
	{
		static_for<0, N>()(f);
	}
	
	scalar_type const operator[](int i) const
	{
		return data[i];
	}
	
	scalar_type& operator[](int i)
	{
		return data[i];
	}
	
#define DEF_OP_UNARY_SCALAR(op) \
	vector_type& operator op(scalar_type s) \
	{ \
		iterate([&](int i){ data[i] op s; }); \
		return *this; \
	}
	
	DEF_OP_UNARY_SCALAR(+=)
	DEF_OP_UNARY_SCALAR(-=)
	DEF_OP_UNARY_SCALAR(*=)
	DEF_OP_UNARY_SCALAR(/=)
	
#define DEF_OP_UNARY_VECTOR(op) \
	vector_type& operator op(const vector_type &o) \
	{ \
		iterate([&](int i){ data[i] op o[i]; }); \
		return *this; \
	}
	
	DEF_OP_UNARY_VECTOR(+=)
	DEF_OP_UNARY_VECTOR(-=)
	DEF_OP_UNARY_VECTOR(*=)
	DEF_OP_UNARY_VECTOR(/=)
};

// NOTE: the -> return declaration is preferred 
// because it allows for strong typing
// meaning that the functions/operators 
// template arguments are less
// likely to accept invalid types 

// NOTE: all the binary operators use an
// implementation that will hopefully
// trigger NRVO

#define DEF_OP_BINARY(op, impl_op, a_type, b_type) \
	template<class V> \
	inline auto operator op(a_type a, b_type b) -> typename V::vector_type \
	{ \
		V out = a; \
		out impl_op b; \
		return out; \
	}

DEF_OP_BINARY(+, +=, const V&, const V&)
DEF_OP_BINARY(-, -=, const V&, const V&)
DEF_OP_BINARY(*, *=, const V&, const V&)
DEF_OP_BINARY(/, /=, const V&, const V&)
DEF_OP_BINARY(+, +=, const V&, typename V::scalar_type)
DEF_OP_BINARY(+, +=, typename V::scalar_type, const V&)
DEF_OP_BINARY(-, -=, const V&, typename V::scalar_type)
DEF_OP_BINARY(-, -=, typename V::scalar_type, const V&)
DEF_OP_BINARY(*, *=, const V&, typename V::scalar_type)
DEF_OP_BINARY(*, *=, typename V::scalar_type, const V&)
DEF_OP_BINARY(/, /=, const V&, typename V::scalar_type)
DEF_OP_BINARY(/, /=, typename V::scalar_type, const V&)

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
	V::iterate([&](int i){
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

int main ()
{
	typedef vector<float, 4> vec4;
	typedef vector<float, 3> vec3;
	typedef vector<float, 2> vec2;
	
	vec3 a(vec2(1, 2), 3);
	vec3 b(4, vec2(5, 6));
	vec3 c = a - b; //normalize(cross(a, b));
	vec2 i = a.xy;

	printf ("%f %f %f\n", c.x, c.y, c.z);
	printf ("%f\n", length (c));
	
	return 0;
}