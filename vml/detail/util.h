#pragma once

#ifdef _MSC_VER
#define _MSC_FIX_EBO __declspec(empty_bases) // https://blogs.msdn.microsoft.com/vcblog/2016/03/30/optimizing-the-layout-of-empty-base-classes-in-vs2015-update-2-3/
#else
#define _MSC_FIX_EBO
#endif

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

template <class T>
constexpr size_t get_size()
{
	if constexpr (std::is_arithmetic<typename remove_cvref<T>::type>::value) {
		return 1;
	} else {
		return remove_cvref<T>::type::num_components;
	}
}

} }