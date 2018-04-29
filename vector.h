#pragma once

#include <utility>

#include "static_for.h"

namespace vmath {

template<typename T, int N>
struct vector;

// * this is a quick way of alowing to set
// an element by index only when the index is valid
// it is done like this with a helper struct
// because function template specialization is
// not allowed - see: http://www.gotw.ca/publications/mill17.htm
template<int i, int j>
struct swizzle_index
{
	template<class Dest, class Src>
	static void set(Dest &dest, const Src &src)
	{
		dest[i] = src[j];
	}
};

template<int i>
struct swizzle_index<i, -1>
{
	template<class Dest, class Src>
	static void set(Dest &, const Src &)
	{
	}
};

template<
	typename T, int P, int N,
	int X = -1, int Y = -1, int Z = -1, int W = -1>
struct swizzler
{
	typedef vector<T, P> parent_type;
	typedef vector<T, N> vector_type;

	T data[N];

	operator vector_type() const
	{
		vector_type v;
		swizzle_index<0, X>::set(v.data, data);
		swizzle_index<1, Y>::set(v.data, data);
		swizzle_index<2, Z>::set(v.data, data);
		swizzle_index<4, W>::set(v.data, data);
		return v;
	}
};

#include "v_base.h"

template<typename T, int N>
struct vector : public vector_base<T, N>
{
	typedef T scalar_type;
	typedef vector<T, N> vector_type;
	typedef vector_base<T, N> base_type;

	// bring in scope the union member
	using base_type::data;

	vector()
	{
		iterate([&](int i) {
			data[i] = 0;
		});
	}

	vector(scalar_type s)
	{
		iterate([s, this](int i) {
			data[i] = s;
		});
	}

	//TODO: clarify if these are really needed
	vector(const vector_type &) = default;
	vector_type& operator=(const vector &) = default;
	vector(vector_type &&) = default;
	vector_type& operator=(vector &&) = default;

	bool construct_at_index(int &i, T &&arg)
	{
		data[i++] = arg;
		return true;
	}

	template<int HowMany>
	bool construct_at_index(int &i, vector<T, HowMany> &&arg)
	{
		static_for<0, HowMany>()([&](int j) {
			data[i++] = arg.data[j];
		});
		return true;
	}

	template<typename... S>
	explicit vector(S... args)
	{
		static_assert(
			(sizeof...(args) <= N),
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
			{ construct_at_index(i, std::forward<S>(args)) ... }
		);
	}

	template<class Func>
	static void iterate(Func &&f)
	{
		static_for<0, N>()(std::forward<Func>(f));
	}

	scalar_type const operator[](int i) const
	{
		return data[i];
	}

	scalar_type& operator[](int i)
	{
		return data[i];
	}

#include "v_oper.h"
};

#include "v_func.h"

} // namespace