#pragma once

#include <utility>

#include "static_for.h"

namespace vmath {

// we'll have to accept this
// warning C4201 : nonstandard extension used : nameless struct / union
#pragma warning(disable: 4201)

template<typename T, size_t N>
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
	typename T, int N,
	int X = -1, int Y = -1, int Z = -1, int W = -1>
struct swizzler
{
	using vector_type = vector<T, N>;

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

template<typename T, size_t N>
struct vector : public vector_base<T, N>
{
	using scalar_type = T;
	using vector_type = vector<T, N>;
	using base_type = vector_base<T, N>;

	// useful in non-member functions
	static constexpr auto num_components = N;

	// bring in scope the union member
	using base_type::data;

	vector()
	{
		iterate([&](size_t i) {
			data[i] = 0;
		});
	}

	vector(scalar_type s)
	{
		iterate([s, this](size_t i) {
			data[i] = s;
		});
	}

	// not realy needed but good for debugging
	// ex: switch to `delete` to see where they are needed
	vector(const vector_type &) = default;
	vector_type& operator=(const vector &) = default;
	vector(vector_type &&) = default;
	vector_type& operator=(vector &&) = default;

	bool construct_at_index(size_t &i, scalar_type arg)
	{
		data[i++] = arg;
		return true; // dummy return, just because it wil be called in a {} init list
	}

	template<typename Other, size_t Other_N>
	bool construct_at_index(size_t &i, vector<Other, Other_N> &&arg)
	{
		constexpr auto count = std::min(N, Other_N);
		static_for<0, count>()([&](size_t j) {
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

		// dummy forwarding structure
		struct constructor
		{
			constructor(...) {}
		};

		size_t i = 0;
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

	scalar_type const operator[](size_t i) const
	{
		return data[i];
	}

	scalar_type& operator[](size_t i)
	{
		return data[i];
	}

#include "v_oper.h"
};

#include "v_func.h"

} // namespace
