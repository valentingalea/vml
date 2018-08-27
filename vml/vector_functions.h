#pragma once

// depends on "vector.h"
// depends on "detail/traits.h" 

// NOTE: need to call decay() so promoted scalars (vec1's) go back down again
// otherwise binary operators will get confused
#define MAKE_LIB_FUNC(name) \
template<class... Args> \
inline auto name(Args&&... args) -> \
	decltype(::vml::detail::decay( \
		::vml::traits::promote_to_vec<Args...>::type::lib_##name(std::forward<Args>(args)...) \
	)) \
{ \
	return ::vml::traits::promote_to_vec<Args...>::type:: \
		lib_##name(std::forward<Args>(args)...); \
}

MAKE_LIB_FUNC(radians)
MAKE_LIB_FUNC(degrees)
MAKE_LIB_FUNC(sin)
MAKE_LIB_FUNC(cos)
MAKE_LIB_FUNC(tan)
MAKE_LIB_FUNC(asin)
MAKE_LIB_FUNC(acos)
MAKE_LIB_FUNC(atan)

MAKE_LIB_FUNC(pow)
MAKE_LIB_FUNC(exp)
MAKE_LIB_FUNC(log)
MAKE_LIB_FUNC(exp2)
MAKE_LIB_FUNC(log2)
MAKE_LIB_FUNC(sqrt)
MAKE_LIB_FUNC(inversesqrt)

MAKE_LIB_FUNC(abs)
MAKE_LIB_FUNC(sign)
MAKE_LIB_FUNC(floor)
MAKE_LIB_FUNC(trunc)
MAKE_LIB_FUNC(ceil)
MAKE_LIB_FUNC(fract)
MAKE_LIB_FUNC(mod)
MAKE_LIB_FUNC(min)
MAKE_LIB_FUNC(max)
MAKE_LIB_FUNC(clamp)
MAKE_LIB_FUNC(mix)
MAKE_LIB_FUNC(step)
MAKE_LIB_FUNC(smoothstep)

MAKE_LIB_FUNC(length)
MAKE_LIB_FUNC(distance)
MAKE_LIB_FUNC(normalize)
MAKE_LIB_FUNC(dot)
MAKE_LIB_FUNC(cross)
MAKE_LIB_FUNC(faceforward)
MAKE_LIB_FUNC(reflect)
MAKE_LIB_FUNC(refract)

MAKE_LIB_FUNC(lessThan)
MAKE_LIB_FUNC(lessThanEqual)
MAKE_LIB_FUNC(greaterThan)
MAKE_LIB_FUNC(greaterThanEqual)
MAKE_LIB_FUNC(equal)
MAKE_LIB_FUNC(notEqual)
MAKE_LIB_FUNC(any)
MAKE_LIB_FUNC(all)
MAKE_LIB_FUNC(_not)

MAKE_LIB_FUNC(dFdx)
MAKE_LIB_FUNC(dFdy)
MAKE_LIB_FUNC(fwidth)

#undef MAKE_LIB_FUNC