#pragma once

namespace swizzle { namespace detail
{

template<typename vector_type, typename T, size_t N, size_t... indices>
struct swizzler
{
	T data[N]; // N might differ from vector_type::num_components ex: .xxxx from vec3

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

} } // namespace swizzle::detail