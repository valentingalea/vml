#pragma once

template<
    int Begin, 
    int End>
struct static_for
{
	template<class Func>
	void operator ()(Func &&f) //TODO: constrain with enable_if callable
	{
		f(Begin);
		if (Begin < End) {
			static_for<Begin + 1, End>()(f);
		}
	}
};

template<int N>
struct static_for<N, N>
{
	template<class Func>
	constexpr void operator ()(Func &&f)
	{
	}
};