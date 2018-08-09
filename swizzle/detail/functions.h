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
		out /= length(v);
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

	friend vector_type cross(const vector_type &a, const vector_type &b)
	{
		static_assert(N == 3, "cross product only works for vec3");

		return vector_type(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
		);
	}
};

} } // namespace swizzle::detail