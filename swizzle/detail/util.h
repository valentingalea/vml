#pragma once

namespace swizzle { namespace detail
{

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
auto decay(T&& t) -> decltype(t.decay())
{
	return t.decay();
}

template <class T>
typename std::enable_if<
	std::is_scalar<typename std::remove_reference<T>::type >::value,
	T>::type
	decay(T&& t)
{
	return t;
}

} }