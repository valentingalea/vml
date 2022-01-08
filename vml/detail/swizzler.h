#pragma once

namespace vml { namespace detail
{

template<typename vector_type, typename T, size_t N, size_t... indices>
struct swizzler
{
	static constexpr auto num_components = sizeof...(indices); // same as vector_type's

	T data[N]; // N might differ from num_components ex: .xxxx from vec3

	vector_type decay() const
	{
		vector_type vec;
		assign_across(vec, 0, indices...);
		return vec;
	}

	operator vector_type() const
	{
		return decay();
	}

	operator vector_type()
	{
		return decay();
	}

	swizzler& operator=(const vector_type &vec)
	{
		assign_across(vec, 0, indices...);
		return *this;
	}

	//TODO: constrain the assignment only when indices are different

	using self_type = swizzler;
	using scalar_type = T;
#define Is indices
#define HAS_UNARY_MUL
#define OMIT_NEG_OP
#include "unary_ops.h"

	vector_type operator -() const
	{
		return vector_type((-data[indices])...);
	}
#undef OMIT_NEG_OP

private:
	template<typename... Indices>
	void assign_across(vector_type &vec, size_t i, Indices ...swizz_i) const
	{
		((vec[i++] = data[swizz_i]), ...);
	}

	template<typename... Indices>
	void assign_across(const vector_type &vec, size_t i, Indices ...swizz_i)
	{
		((data[swizz_i] = vec[i++]), ...);
	}
};

} } // namespace vml::detail