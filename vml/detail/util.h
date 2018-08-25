#pragma once

namespace vml {

template<typename T, size_t... Ns>
struct vector;

template<typename T, size_t N, template<size_t...> class swizzler_wrapper>
struct vector_base;

namespace detail {

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
	std::is_scalar<typename std::remove_reference<T>::type >::value, T>::type
decay(T&& t)
{
	return t;
}

template<typename V, typename... Ts>
constexpr bool converts_to()
{
	return (std::is_convertible<Ts, V>::value || ...);
}

template<typename, size_t>
struct vec_equiv;

template<typename T>
struct vec_equiv<T, 1>
{
	using type = vector<T, 0>;
};

template<typename T>
struct vec_equiv<T, 2>
{
	using type = vector<T, 0, 1>;
};

template<typename T>
struct vec_equiv<T, 3>
{
	using type = vector<T, 0, 1, 2>;
};

template<typename T>
struct vec_equiv<T, 4>
{
	using type = vector<T, 0, 1, 2, 3>;
};

template<typename T, size_t... Ns>
struct vector_base_selector
{
	static_assert(sizeof...(Ns), "must have at least 1 component");

	template<size_t... indices> // 0 for x (or r), 1 for y (or g), etc 
	struct swizzler_wrapper_factory
	{
		// NOTE: need to pass the equivalent vector type
		// this can be different than the current 'host' vector
		// for ex:
		// .xy is vec2 that is part of a vec3
		// .xy is also vec2 but part of a vec4
		// they need to be same underlying type
		using type = detail::swizzler<
			typename vec_equiv<T, sizeof...(indices)>::type, T, sizeof...(Ns), indices...>;
	};

	template<size_t x>
	struct swizzler_wrapper_factory<x>
	{
		using type = T; // one component vectors are just scalars
	};

	using base_type = vector_base<T, sizeof...(Ns), swizzler_wrapper_factory>;
};

} }