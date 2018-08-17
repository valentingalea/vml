#pragma once

#include <cmath>

namespace swizzle { namespace detail
{

template<typename vector_type, typename scalar_type, size_t... Ns>
struct builtin_func_lib
{
	friend scalar_type length(const vector_type &v)
	{
		return std::sqrt(v.dot(v, v));
	}

	friend vector_type normalize(const vector_type &v)
	{
		vector_type out = v;
		out /= length(v);
		return out;
	}

	scalar_type dot(const vector_type &a, const vector_type &b) const
	{
		scalar_type sum = 0;
		((sum += a[Ns] * b[Ns]), ...);
		return sum;
	}

	friend scalar_type dot(const vector_type &a, const vector_type &b)
	{
		return a.dot(a, b);
	}

	friend vector_type cross(const vector_type &a, const vector_type &b)
	{
		static_assert(vector_type::num_components == 3, "cross product only works for vec3");

		return vector_type(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
		);
	}
};

} } // namespace swizzle::detail