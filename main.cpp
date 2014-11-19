#include <cstdio>
#include <cmath>
#include <static_for.h>

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
	};
};

// TODO: more specialisation 

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
	
	// dummy structure that is only used
	// to initialise it with an std::initalizer_list
	// where we will get the chance to run
	// special code for each list element 
	struct constructor
	{
		template<typename... Args>
		constructor(Args... args) {}
	};
	
	// TODO: find a way to init vector elem using this func
	//static void elem_set(scalar_type &arg)

	template<typename... S>
	explicit vector (S... args)
	{
		static_assert(
		  (sizeof...(args) <= num_components),
		  "mismatch number of vector init arguments");
		
		int i = 0;
		constructor(
		// the use of {} guarantee left to right processing 
		// the , 1 is a trick needed because you actually
		// need to put something in each element
		// so comma operator is used to first run some
		// code and then to always return 1
		  { (data[i++] = args, 1) ... }
		);
	}
	
	scalar_type const operator[](int i) const
	{
		return data[i];
	}
	
	template<class Func>
	static void iterate(Func f)
	{
		static_for<0, N>()(f);
	}
};

template<class V>
inline auto length(const V &v) -> typename V::scalar_type //decltype (v[0])
{
	return std::sqrt(dot(v, v));
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
	typedef vector<float, 3> vec3;
	typedef vector<float, 2> vec2;
	
	vec3 a(1,0,0);
	vec3 b(0,1,0);
	auto c = cross(b, a);

	
	printf ("%f %f %f", c.x, c.y, c.z);
	
	return 0;
}