#pragma once

#include <cmath>

namespace vml { namespace detail
{

template<typename vector_type, typename scalar_type, size_t... Ns>
struct builtin_func_lib
{
#ifdef MSVC_VER
#define INL __forceinline
#else
#define INL __attribute__((always_inline))
#endif

	using vector_arg_type = const vector_type &;

	friend scalar_type length(const vector_type &v)
	{
		return std::sqrt(v.dot(v, v));
	}

	friend scalar_type distance(vector_arg_type p0, const vector_arg_type p1)
	{
		return length(p0 - p1);
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

	friend vector_type faceforward(vector_arg_type N, vector_arg_type I, vector_arg_type Nref)
	{
		return (N.dot(Nref, I) < scalar_type(0) ? N : (-N));
	}

	friend vector_type reflect(vector_arg_type I, vector_arg_type N)
	{
		return (I - scalar_type(2) * N.dot(I, N) * N);
	}

	friend vector_type refract(vector_arg_type I, vector_arg_type N, scalar_type eta)
	{
		constexpr auto one = scalar_type(1);
		constexpr auto zero = scalar_type(0);
		auto k = one - eta * eta * (one - N.dot(N, I) * N.dot(N, I));
		if (k < zero) {
			return vector_type();
		} else {
			return eta * I - (eta * N.dot(N, I) + sqrt(k)) * N;
		}
	}

#undef INL
};

} } // namespace vml::detail