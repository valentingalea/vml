#pragma once

namespace vml { namespace detail {

struct nothing {};

template<size_t Begin, size_t End>
struct static_for
{
	template<class Func>
	constexpr void operator ()(Func &&f)
	{
		f(Begin);
		static_for<Begin + 1, End>()(std::forward<Func>(f));
	}
};

template<size_t N>
struct static_for<N, N>
{
	template<class Func>
	constexpr void operator ()(Func &&)
	{
	}
};

template <class T>
constexpr auto decay(T&& t) -> decltype(t.decay())
{
	return t.decay();
}

template <class T>
constexpr typename std::enable_if<
	std::is_arithmetic<typename std::remove_reference<T>::type >::value, T>::type
decay(T&& t)
{
	return t;
}

// will be available in C++20
template<class T>
struct remove_cvref
{
	using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

} }