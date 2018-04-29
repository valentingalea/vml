#pragma once

template<
    int Begin, 
    int End>
struct static_for
{
	template<class Func>
	void operator ()(Func &&f) //TODO: constrain with `enable_if callable`
	{
		f(Begin);

//TODO: a cleaner way is `if constexpr`
#pragma warning(push)
#pragma warning(disable:4127) // conditional expression is constant
		if (Begin < End) {
#pragma warning(pop)
			static_for<Begin + 1, End>()(f);
		}
	}
};

template<int N>
struct static_for<N, N>
{
	template<class Func>
	constexpr void operator ()(Func &&)
	{
	}
};