#pragma once

#include <cmath>

namespace swizzle { namespace detail
{

template<typename vector_type, typename scalar_type, size_t N>
struct builtin_func_lib
{
	friend scalar_type length(const vector_type &v)
	{
		return std::sqrt(dot(v, v));
	}

	friend vector_type normalize(const vector_type &v)
	{
		vector_type out = v;
	//	out /= length(v);
		return out;
	}

	friend scalar_type dot(const vector_type &a, const vector_type &b)
	{
		scalar_type sum = 0;

		vector_type::iterate([&](size_t i) {
			sum += a[i] * b[i];
		});

		return sum;
	}
};

} } // namespace swizzle::detail