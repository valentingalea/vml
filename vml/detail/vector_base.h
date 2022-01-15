#pragma once

// only MSVS has a problem with this
// warning C4201 : nonstandard extension used : nameless struct / union
#pragma warning(disable: 4201)

namespace vml {

template<typename T, size_t... Ns>
struct vector;

namespace detail {

template<typename T, size_t N, template<size_t...> class swizzler_wrapper>
struct vector_base;

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
	static_assert(sizeof...(Ns) >= 1, "must have at least 1 component");

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

template<typename T, template<size_t...> class swizzler_wrapper>
struct vector_base<T, 1, swizzler_wrapper>
{
	union
	{
		T data[1];

		struct
		{
			typename swizzler_wrapper<0>::type x;
		};
		struct
		{
			typename swizzler_wrapper<0>::type r;
		};
		struct
		{
			typename swizzler_wrapper<0>::type s;
		};

		typename swizzler_wrapper<0, 0>::type xx, rr, ss;
		typename swizzler_wrapper<0, 0, 0>::type xxx, rrr, sss;
		typename swizzler_wrapper<0, 0, 0, 0>::type xxxx, rrrr, ssss;
	};
};

template<typename T, template<size_t...> class swizzler_wrapper>
struct vector_base<T, 2, swizzler_wrapper>
{
	union
	{
		T data[2];

		struct
		{
			typename swizzler_wrapper<0>::type x;
			typename swizzler_wrapper<1>::type y;
		};
		struct
		{
			typename swizzler_wrapper<0>::type r;
			typename swizzler_wrapper<1>::type g;
		};
		struct
		{
			typename swizzler_wrapper<0>::type s;
			typename swizzler_wrapper<1>::type t;
		};

		typename swizzler_wrapper<0, 0>::type xx, rr, ss;
		typename swizzler_wrapper<0, 1>::type xy, rg, st;
		typename swizzler_wrapper<1, 0>::type yx, gr, ts;
		typename swizzler_wrapper<1, 1>::type yy, gg, tt;
		typename swizzler_wrapper<0, 0, 0>::type xxx, rrr, sss;
		typename swizzler_wrapper<0, 0, 1>::type xxy, rrg, sst;
		typename swizzler_wrapper<0, 1, 0>::type xyx, rgr, sts;
		typename swizzler_wrapper<0, 1, 1>::type xyy, rgg, stt;
		typename swizzler_wrapper<1, 0, 0>::type yxx, grr, tss;
		typename swizzler_wrapper<1, 0, 1>::type yxy, grg, tst;
		typename swizzler_wrapper<1, 1, 0>::type yyx, ggr, tts;
		typename swizzler_wrapper<1, 1, 1>::type yyy, ggg, ttt;
		typename swizzler_wrapper<0, 0, 0, 0>::type xxxx, rrrr, ssss;
		typename swizzler_wrapper<0, 0, 0, 1>::type xxxy, rrrg, ssst;
		typename swizzler_wrapper<0, 0, 1, 0>::type xxyx, rrgr, ssts;
		typename swizzler_wrapper<0, 0, 1, 1>::type xxyy, rrgg, sstt;
		typename swizzler_wrapper<0, 1, 0, 0>::type xyxx, rgrr, stss;
		typename swizzler_wrapper<0, 1, 0, 1>::type xyxy, rgrg, stst;
		typename swizzler_wrapper<0, 1, 1, 0>::type xyyx, rggr, stts;
		typename swizzler_wrapper<0, 1, 1, 1>::type xyyy, rggg, sttt;
		typename swizzler_wrapper<1, 0, 0, 0>::type yxxx, grrr, tsss;
		typename swizzler_wrapper<1, 0, 0, 1>::type yxxy, grrg, tsst;
		typename swizzler_wrapper<1, 0, 1, 0>::type yxyx, grgr, tsts;
		typename swizzler_wrapper<1, 0, 1, 1>::type yxyy, grgg, tstt;
		typename swizzler_wrapper<1, 1, 0, 0>::type yyxx, ggrr, ttss;
		typename swizzler_wrapper<1, 1, 0, 1>::type yyxy, ggrg, ttst;
		typename swizzler_wrapper<1, 1, 1, 0>::type yyyx, gggr, ttts;
		typename swizzler_wrapper<1, 1, 1, 1>::type yyyy, gggg, tttt;
	};
};

template<typename T, template<size_t...> class swizzler_wrapper>
struct vector_base<T, 3, swizzler_wrapper>
{
	union
	{
		T data[3];

		struct
		{
			typename swizzler_wrapper<0>::type x;
			typename swizzler_wrapper<1>::type y;
			typename swizzler_wrapper<2>::type z;
		};
		struct
		{
			typename swizzler_wrapper<0>::type r;
			typename swizzler_wrapper<1>::type g;
			typename swizzler_wrapper<2>::type b;
		};
		struct
		{
			typename swizzler_wrapper<0>::type s;
			typename swizzler_wrapper<1>::type t;
			typename swizzler_wrapper<2>::type p;
		};

		typename swizzler_wrapper<0, 0>::type xx, rr, ss;
		typename swizzler_wrapper<0, 1>::type xy, rg, st;
		typename swizzler_wrapper<0, 2>::type xz, rb, sp;
		typename swizzler_wrapper<1, 0>::type yx, gr, ts;
		typename swizzler_wrapper<1, 1>::type yy, gg, tt;
		typename swizzler_wrapper<1, 2>::type yz, gb, tp;
		typename swizzler_wrapper<2, 0>::type zx, br, ps;
		typename swizzler_wrapper<2, 1>::type zy, bg, pt;
		typename swizzler_wrapper<2, 2>::type zz, bb, pp;
		typename swizzler_wrapper<0, 0, 0>::type xxx, rrr, sss;
		typename swizzler_wrapper<0, 0, 1>::type xxy, rrg, sst;
		typename swizzler_wrapper<0, 0, 2>::type xxz, rrb, ssp;
		typename swizzler_wrapper<0, 1, 0>::type xyx, rgr, sts;
		typename swizzler_wrapper<0, 1, 1>::type xyy, rgg, stt;
		typename swizzler_wrapper<0, 1, 2>::type xyz, rgb, stp;
		typename swizzler_wrapper<0, 2, 0>::type xzx, rbr, sps;
		typename swizzler_wrapper<0, 2, 1>::type xzy, rbg, spt;
		typename swizzler_wrapper<0, 2, 2>::type xzz, rbb, spp;
		typename swizzler_wrapper<1, 0, 0>::type yxx, grr, tss;
		typename swizzler_wrapper<1, 0, 1>::type yxy, grg, tst;
		typename swizzler_wrapper<1, 0, 2>::type yxz, grb, tsp;
		typename swizzler_wrapper<1, 1, 0>::type yyx, ggr, tts;
		typename swizzler_wrapper<1, 1, 1>::type yyy, ggg, ttt;
		typename swizzler_wrapper<1, 1, 2>::type yyz, ggb, ttp;
		typename swizzler_wrapper<1, 2, 0>::type yzx, gbr, tps;
		typename swizzler_wrapper<1, 2, 1>::type yzy, gbg, tpt;
		typename swizzler_wrapper<1, 2, 2>::type yzz, gbb, tpp;
		typename swizzler_wrapper<2, 0, 0>::type zxx, brr, pss;
		typename swizzler_wrapper<2, 0, 1>::type zxy, brg, pst;
		typename swizzler_wrapper<2, 0, 2>::type zxz, brb, psp;
		typename swizzler_wrapper<2, 1, 0>::type zyx, bgr, pts;
		typename swizzler_wrapper<2, 1, 1>::type zyy, bgg, ptt;
		typename swizzler_wrapper<2, 1, 2>::type zyz, bgb, ptp;
		typename swizzler_wrapper<2, 2, 0>::type zzx, bbr, pps;
		typename swizzler_wrapper<2, 2, 1>::type zzy, bbg, ppt;
		typename swizzler_wrapper<2, 2, 2>::type zzz, bbb, ppp;
		typename swizzler_wrapper<0, 0, 0, 0>::type xxxx, rrrr, ssss;
		typename swizzler_wrapper<0, 0, 0, 1>::type xxxy, rrrg, ssst;
		typename swizzler_wrapper<0, 0, 0, 2>::type xxxz, rrrb, sssp;
		typename swizzler_wrapper<0, 0, 1, 0>::type xxyx, rrgr, ssts;
		typename swizzler_wrapper<0, 0, 1, 1>::type xxyy, rrgg, sstt;
		typename swizzler_wrapper<0, 0, 1, 2>::type xxyz, rrgb, sstp;
		typename swizzler_wrapper<0, 0, 2, 0>::type xxzx, rrbr, ssps;
		typename swizzler_wrapper<0, 0, 2, 1>::type xxzy, rrbg, sspt;
		typename swizzler_wrapper<0, 0, 2, 2>::type xxzz, rrbb, sspp;
		typename swizzler_wrapper<0, 1, 0, 0>::type xyxx, rgrr, stss;
		typename swizzler_wrapper<0, 1, 0, 1>::type xyxy, rgrg, stst;
		typename swizzler_wrapper<0, 1, 0, 2>::type xyxz, rgrb, stsp;
		typename swizzler_wrapper<0, 1, 1, 0>::type xyyx, rggr, stts;
		typename swizzler_wrapper<0, 1, 1, 1>::type xyyy, rggg, sttt;
		typename swizzler_wrapper<0, 1, 1, 2>::type xyyz, rggb, sttp;
		typename swizzler_wrapper<0, 1, 2, 0>::type xyzx, rgbr, stps;
		typename swizzler_wrapper<0, 1, 2, 1>::type xyzy, rgbg, stpt;
		typename swizzler_wrapper<0, 1, 2, 2>::type xyzz, rgbb, stpp;
		typename swizzler_wrapper<0, 2, 0, 0>::type xzxx, rbrr, spss;
		typename swizzler_wrapper<0, 2, 0, 1>::type xzxy, rbrg, spst;
		typename swizzler_wrapper<0, 2, 0, 2>::type xzxz, rbrb, spsp;
		typename swizzler_wrapper<0, 2, 1, 0>::type xzyx, rbgr, spts;
		typename swizzler_wrapper<0, 2, 1, 1>::type xzyy, rbgg, sptt;
		typename swizzler_wrapper<0, 2, 1, 2>::type xzyz, rbgb, sptp;
		typename swizzler_wrapper<0, 2, 2, 0>::type xzzx, rbbr, spps;
		typename swizzler_wrapper<0, 2, 2, 1>::type xzzy, rbbg, sppt;
		typename swizzler_wrapper<0, 2, 2, 2>::type xzzz, rbbb, sppp;
		typename swizzler_wrapper<1, 0, 0, 0>::type yxxx, grrr, tsss;
		typename swizzler_wrapper<1, 0, 0, 1>::type yxxy, grrg, tsst;
		typename swizzler_wrapper<1, 0, 0, 2>::type yxxz, grrb, tssp;
		typename swizzler_wrapper<1, 0, 1, 0>::type yxyx, grgr, tsts;
		typename swizzler_wrapper<1, 0, 1, 1>::type yxyy, grgg, tstt;
		typename swizzler_wrapper<1, 0, 1, 2>::type yxyz, grgb, tstp;
		typename swizzler_wrapper<1, 0, 2, 0>::type yxzx, grbr, tsps;
		typename swizzler_wrapper<1, 0, 2, 1>::type yxzy, grbg, tspt;
		typename swizzler_wrapper<1, 0, 2, 2>::type yxzz, grbb, tspp;
		typename swizzler_wrapper<1, 1, 0, 0>::type yyxx, ggrr, ttss;
		typename swizzler_wrapper<1, 1, 0, 1>::type yyxy, ggrg, ttst;
		typename swizzler_wrapper<1, 1, 0, 2>::type yyxz, ggrb, ttsp;
		typename swizzler_wrapper<1, 1, 1, 0>::type yyyx, gggr, ttts;
		typename swizzler_wrapper<1, 1, 1, 1>::type yyyy, gggg, tttt;
		typename swizzler_wrapper<1, 1, 1, 2>::type yyyz, gggb, tttp;
		typename swizzler_wrapper<1, 1, 2, 0>::type yyzx, ggbr, ttps;
		typename swizzler_wrapper<1, 1, 2, 1>::type yyzy, ggbg, ttpt;
		typename swizzler_wrapper<1, 1, 2, 2>::type yyzz, ggbb, ttpp;
		typename swizzler_wrapper<1, 2, 0, 0>::type yzxx, gbrr, tpss;
		typename swizzler_wrapper<1, 2, 0, 1>::type yzxy, gbrg, tpst;
		typename swizzler_wrapper<1, 2, 0, 2>::type yzxz, gbrb, tpsp;
		typename swizzler_wrapper<1, 2, 1, 0>::type yzyx, gbgr, tpts;
		typename swizzler_wrapper<1, 2, 1, 1>::type yzyy, gbgg, tptt;
		typename swizzler_wrapper<1, 2, 1, 2>::type yzyz, gbgb, tptp;
		typename swizzler_wrapper<1, 2, 2, 0>::type yzzx, gbbr, tpps;
		typename swizzler_wrapper<1, 2, 2, 1>::type yzzy, gbbg, tppt;
		typename swizzler_wrapper<1, 2, 2, 2>::type yzzz, gbbb, tppp;
		typename swizzler_wrapper<2, 0, 0, 0>::type zxxx, brrr, psss;
		typename swizzler_wrapper<2, 0, 0, 1>::type zxxy, brrg, psst;
		typename swizzler_wrapper<2, 0, 0, 2>::type zxxz, brrb, pssp;
		typename swizzler_wrapper<2, 0, 1, 0>::type zxyx, brgr, psts;
		typename swizzler_wrapper<2, 0, 1, 1>::type zxyy, brgg, pstt;
		typename swizzler_wrapper<2, 0, 1, 2>::type zxyz, brgb, pstp;
		typename swizzler_wrapper<2, 0, 2, 0>::type zxzx, brbr, psps;
		typename swizzler_wrapper<2, 0, 2, 1>::type zxzy, brbg, pspt;
		typename swizzler_wrapper<2, 0, 2, 2>::type zxzz, brbb, pspp;
		typename swizzler_wrapper<2, 1, 0, 0>::type zyxx, bgrr, ptss;
		typename swizzler_wrapper<2, 1, 0, 1>::type zyxy, bgrg, ptst;
		typename swizzler_wrapper<2, 1, 0, 2>::type zyxz, bgrb, ptsp;
		typename swizzler_wrapper<2, 1, 1, 0>::type zyyx, bggr, ptts;
		typename swizzler_wrapper<2, 1, 1, 1>::type zyyy, bggg, pttt;
		typename swizzler_wrapper<2, 1, 1, 2>::type zyyz, bggb, pttp;
		typename swizzler_wrapper<2, 1, 2, 0>::type zyzx, bgbr, ptps;
		typename swizzler_wrapper<2, 1, 2, 1>::type zyzy, bgbg, ptpt;
		typename swizzler_wrapper<2, 1, 2, 2>::type zyzz, bgbb, ptpp;
		typename swizzler_wrapper<2, 2, 0, 0>::type zzxx, bbrr, ppss;
		typename swizzler_wrapper<2, 2, 0, 1>::type zzxy, bbrg, ppst;
		typename swizzler_wrapper<2, 2, 0, 2>::type zzxz, bbrb, ppsp;
		typename swizzler_wrapper<2, 2, 1, 0>::type zzyx, bbgr, ppts;
		typename swizzler_wrapper<2, 2, 1, 1>::type zzyy, bbgg, pptt;
		typename swizzler_wrapper<2, 2, 1, 2>::type zzyz, bbgb, pptp;
		typename swizzler_wrapper<2, 2, 2, 0>::type zzzx, bbbr, ppps;
		typename swizzler_wrapper<2, 2, 2, 1>::type zzzy, bbbg, pppt;
		typename swizzler_wrapper<2, 2, 2, 2>::type zzzz, bbbb, pppp;
	};
};

template<typename T, template<size_t...> class swizzler_wrapper>
struct vector_base<T, 4, swizzler_wrapper>
{
	union
	{
		T data[4];

		struct
		{
			typename swizzler_wrapper<0>::type x;
			typename swizzler_wrapper<1>::type y;
			typename swizzler_wrapper<2>::type z;
			typename swizzler_wrapper<3>::type w;
		};
		struct
		{
			typename swizzler_wrapper<0>::type r;
			typename swizzler_wrapper<1>::type g;
			typename swizzler_wrapper<2>::type b;
			typename swizzler_wrapper<3>::type a;
		};
		struct
		{
			typename swizzler_wrapper<0>::type s;
			typename swizzler_wrapper<1>::type t;
			typename swizzler_wrapper<2>::type p;
			typename swizzler_wrapper<3>::type q;
		};

		typename swizzler_wrapper<0, 0>::type xx, rr, ss;
		typename swizzler_wrapper<0, 1>::type xy, rg, st;
		typename swizzler_wrapper<0, 2>::type xz, rb, sp;
		typename swizzler_wrapper<0, 3>::type xw, ra, sq;
		typename swizzler_wrapper<1, 0>::type yx, gr, ts;
		typename swizzler_wrapper<1, 1>::type yy, gg, tt;
		typename swizzler_wrapper<1, 2>::type yz, gb, tp;
		typename swizzler_wrapper<1, 3>::type yw, ga, tq;
		typename swizzler_wrapper<2, 0>::type zx, br, ps;
		typename swizzler_wrapper<2, 1>::type zy, bg, pt;
		typename swizzler_wrapper<2, 2>::type zz, bb, pp;
		typename swizzler_wrapper<2, 3>::type zw, ba, pq;
		typename swizzler_wrapper<3, 0>::type wx, ar, qs;
		typename swizzler_wrapper<3, 1>::type wy, ag, qt;
		typename swizzler_wrapper<3, 2>::type wz, ab, qp;
		typename swizzler_wrapper<3, 3>::type ww, aa, qq;
		typename swizzler_wrapper<0, 0, 0>::type xxx, rrr, sss;
		typename swizzler_wrapper<0, 0, 1>::type xxy, rrg, sst;
		typename swizzler_wrapper<0, 0, 2>::type xxz, rrb, ssp;
		typename swizzler_wrapper<0, 0, 3>::type xxw, rra, ssq;
		typename swizzler_wrapper<0, 1, 0>::type xyx, rgr, sts;
		typename swizzler_wrapper<0, 1, 1>::type xyy, rgg, stt;
		typename swizzler_wrapper<0, 1, 2>::type xyz, rgb, stp;
		typename swizzler_wrapper<0, 1, 3>::type xyw, rga, stq;
		typename swizzler_wrapper<0, 2, 0>::type xzx, rbr, sps;
		typename swizzler_wrapper<0, 2, 1>::type xzy, rbg, spt;
		typename swizzler_wrapper<0, 2, 2>::type xzz, rbb, spp;
		typename swizzler_wrapper<0, 2, 3>::type xzw, rba, spq;
		typename swizzler_wrapper<0, 3, 0>::type xwx, rar, sqs;
		typename swizzler_wrapper<0, 3, 1>::type xwy, rag, sqt;
		typename swizzler_wrapper<0, 3, 2>::type xwz, rab, sqp;
		typename swizzler_wrapper<0, 3, 3>::type xww, raa, sqq;
		typename swizzler_wrapper<1, 0, 0>::type yxx, grr, tss;
		typename swizzler_wrapper<1, 0, 1>::type yxy, grg, tst;
		typename swizzler_wrapper<1, 0, 2>::type yxz, grb, tsp;
		typename swizzler_wrapper<1, 0, 3>::type yxw, gra, tsq;
		typename swizzler_wrapper<1, 1, 0>::type yyx, ggr, tts;
		typename swizzler_wrapper<1, 1, 1>::type yyy, ggg, ttt;
		typename swizzler_wrapper<1, 1, 2>::type yyz, ggb, ttp;
		typename swizzler_wrapper<1, 1, 3>::type yyw, gga, ttq;
		typename swizzler_wrapper<1, 2, 0>::type yzx, gbr, tps;
		typename swizzler_wrapper<1, 2, 1>::type yzy, gbg, tpt;
		typename swizzler_wrapper<1, 2, 2>::type yzz, gbb, tpp;
		typename swizzler_wrapper<1, 2, 3>::type yzw, gba, tpq;
		typename swizzler_wrapper<1, 3, 0>::type ywx, gar, tqs;
		typename swizzler_wrapper<1, 3, 1>::type ywy, gag, tqt;
		typename swizzler_wrapper<1, 3, 2>::type ywz, gab, tqp;
		typename swizzler_wrapper<1, 3, 3>::type yww, gaa, tqq;
		typename swizzler_wrapper<2, 0, 0>::type zxx, brr, pss;
		typename swizzler_wrapper<2, 0, 1>::type zxy, brg, pst;
		typename swizzler_wrapper<2, 0, 2>::type zxz, brb, psp;
		typename swizzler_wrapper<2, 0, 3>::type zxw, bra, psq;
		typename swizzler_wrapper<2, 1, 0>::type zyx, bgr, pts;
		typename swizzler_wrapper<2, 1, 1>::type zyy, bgg, ptt;
		typename swizzler_wrapper<2, 1, 2>::type zyz, bgb, ptp;
		typename swizzler_wrapper<2, 1, 3>::type zyw, bga, ptq;
		typename swizzler_wrapper<2, 2, 0>::type zzx, bbr, pps;
		typename swizzler_wrapper<2, 2, 1>::type zzy, bbg, ppt;
		typename swizzler_wrapper<2, 2, 2>::type zzz, bbb, ppp;
		typename swizzler_wrapper<2, 2, 3>::type zzw, bba, ppq;
		typename swizzler_wrapper<2, 3, 0>::type zwx, bar, pqs;
		typename swizzler_wrapper<2, 3, 1>::type zwy, bag, pqt;
		typename swizzler_wrapper<2, 3, 2>::type zwz, bab, pqp;
		typename swizzler_wrapper<2, 3, 3>::type zww, baa, pqq;
		typename swizzler_wrapper<3, 0, 0>::type wxx, arr, qss;
		typename swizzler_wrapper<3, 0, 1>::type wxy, arg, qst;
		typename swizzler_wrapper<3, 0, 2>::type wxz, arb, qsp;
		typename swizzler_wrapper<3, 0, 3>::type wxw, ara, qsq;
		typename swizzler_wrapper<3, 1, 0>::type wyx, agr, qts;
		typename swizzler_wrapper<3, 1, 1>::type wyy, agg, qtt;
		typename swizzler_wrapper<3, 1, 2>::type wyz, agb, qtp;
		typename swizzler_wrapper<3, 1, 3>::type wyw, aga, qtq;
		typename swizzler_wrapper<3, 2, 0>::type wzx, abr, qps;
		typename swizzler_wrapper<3, 2, 1>::type wzy, abg, qpt;
		typename swizzler_wrapper<3, 2, 2>::type wzz, abb, qpp;
		typename swizzler_wrapper<3, 2, 3>::type wzw, aba, qpq;
		typename swizzler_wrapper<3, 3, 0>::type wwx, aar, qqs;
		typename swizzler_wrapper<3, 3, 1>::type wwy, aag, qqt;
		typename swizzler_wrapper<3, 3, 2>::type wwz, aab, qqp;
		typename swizzler_wrapper<3, 3, 3>::type www, aaa, qqq;
		typename swizzler_wrapper<0, 0, 0, 0>::type xxxx, rrrr, ssss;
		typename swizzler_wrapper<0, 0, 0, 1>::type xxxy, rrrg, ssst;
		typename swizzler_wrapper<0, 0, 0, 2>::type xxxz, rrrb, sssp;
		typename swizzler_wrapper<0, 0, 0, 3>::type xxxw, rrra, sssq;
		typename swizzler_wrapper<0, 0, 1, 0>::type xxyx, rrgr, ssts;
		typename swizzler_wrapper<0, 0, 1, 1>::type xxyy, rrgg, sstt;
		typename swizzler_wrapper<0, 0, 1, 2>::type xxyz, rrgb, sstp;
		typename swizzler_wrapper<0, 0, 1, 3>::type xxyw, rrga, sstq;
		typename swizzler_wrapper<0, 0, 2, 0>::type xxzx, rrbr, ssps;
		typename swizzler_wrapper<0, 0, 2, 1>::type xxzy, rrbg, sspt;
		typename swizzler_wrapper<0, 0, 2, 2>::type xxzz, rrbb, sspp;
		typename swizzler_wrapper<0, 0, 2, 3>::type xxzw, rrba, sspq;
		typename swizzler_wrapper<0, 0, 3, 0>::type xxwx, rrar, ssqs;
		typename swizzler_wrapper<0, 0, 3, 1>::type xxwy, rrag, ssqt;
		typename swizzler_wrapper<0, 0, 3, 2>::type xxwz, rrab, ssqp;
		typename swizzler_wrapper<0, 0, 3, 3>::type xxww, rraa, ssqq;
		typename swizzler_wrapper<0, 1, 0, 0>::type xyxx, rgrr, stss;
		typename swizzler_wrapper<0, 1, 0, 1>::type xyxy, rgrg, stst;
		typename swizzler_wrapper<0, 1, 0, 2>::type xyxz, rgrb, stsp;
		typename swizzler_wrapper<0, 1, 0, 3>::type xyxw, rgra, stsq;
		typename swizzler_wrapper<0, 1, 1, 0>::type xyyx, rggr, stts;
		typename swizzler_wrapper<0, 1, 1, 1>::type xyyy, rggg, sttt;
		typename swizzler_wrapper<0, 1, 1, 2>::type xyyz, rggb, sttp;
		typename swizzler_wrapper<0, 1, 1, 3>::type xyyw, rgga, sttq;
		typename swizzler_wrapper<0, 1, 2, 0>::type xyzx, rgbr, stps;
		typename swizzler_wrapper<0, 1, 2, 1>::type xyzy, rgbg, stpt;
		typename swizzler_wrapper<0, 1, 2, 2>::type xyzz, rgbb, stpp;
		typename swizzler_wrapper<0, 1, 2, 3>::type xyzw, rgba, stpq;
		typename swizzler_wrapper<0, 1, 3, 0>::type xywx, rgar, stqs;
		typename swizzler_wrapper<0, 1, 3, 1>::type xywy, rgag, stqt;
		typename swizzler_wrapper<0, 1, 3, 2>::type xywz, rgab, stqp;
		typename swizzler_wrapper<0, 1, 3, 3>::type xyww, rgaa, stqq;
		typename swizzler_wrapper<0, 2, 0, 0>::type xzxx, rbrr, spss;
		typename swizzler_wrapper<0, 2, 0, 1>::type xzxy, rbrg, spst;
		typename swizzler_wrapper<0, 2, 0, 2>::type xzxz, rbrb, spsp;
		typename swizzler_wrapper<0, 2, 0, 3>::type xzxw, rbra, spsq;
		typename swizzler_wrapper<0, 2, 1, 0>::type xzyx, rbgr, spts;
		typename swizzler_wrapper<0, 2, 1, 1>::type xzyy, rbgg, sptt;
		typename swizzler_wrapper<0, 2, 1, 2>::type xzyz, rbgb, sptp;
		typename swizzler_wrapper<0, 2, 1, 3>::type xzyw, rbga, sptq;
		typename swizzler_wrapper<0, 2, 2, 0>::type xzzx, rbbr, spps;
		typename swizzler_wrapper<0, 2, 2, 1>::type xzzy, rbbg, sppt;
		typename swizzler_wrapper<0, 2, 2, 2>::type xzzz, rbbb, sppp;
		typename swizzler_wrapper<0, 2, 2, 3>::type xzzw, rbba, sppq;
		typename swizzler_wrapper<0, 2, 3, 0>::type xzwx, rbar, spqs;
		typename swizzler_wrapper<0, 2, 3, 1>::type xzwy, rbag, spqt;
		typename swizzler_wrapper<0, 2, 3, 2>::type xzwz, rbab, spqp;
		typename swizzler_wrapper<0, 2, 3, 3>::type xzww, rbaa, spqq;
		typename swizzler_wrapper<0, 3, 0, 0>::type xwxx, rarr, sqss;
		typename swizzler_wrapper<0, 3, 0, 1>::type xwxy, rarg, sqst;
		typename swizzler_wrapper<0, 3, 0, 2>::type xwxz, rarb, sqsp;
		typename swizzler_wrapper<0, 3, 0, 3>::type xwxw, rara, sqsq;
		typename swizzler_wrapper<0, 3, 1, 0>::type xwyx, ragr, sqts;
		typename swizzler_wrapper<0, 3, 1, 1>::type xwyy, ragg, sqtt;
		typename swizzler_wrapper<0, 3, 1, 2>::type xwyz, ragb, sqtp;
		typename swizzler_wrapper<0, 3, 1, 3>::type xwyw, raga, sqtq;
		typename swizzler_wrapper<0, 3, 2, 0>::type xwzx, rabr, sqps;
		typename swizzler_wrapper<0, 3, 2, 1>::type xwzy, rabg, sqpt;
		typename swizzler_wrapper<0, 3, 2, 2>::type xwzz, rabb, sqpp;
		typename swizzler_wrapper<0, 3, 2, 3>::type xwzw, raba, sqpq;
		typename swizzler_wrapper<0, 3, 3, 0>::type xwwx, raar, sqqs;
		typename swizzler_wrapper<0, 3, 3, 1>::type xwwy, raag, sqqt;
		typename swizzler_wrapper<0, 3, 3, 2>::type xwwz, raab, sqqp;
		typename swizzler_wrapper<0, 3, 3, 3>::type xwww, raaa, sqqq;
		typename swizzler_wrapper<1, 0, 0, 0>::type yxxx, grrr, tsss;
		typename swizzler_wrapper<1, 0, 0, 1>::type yxxy, grrg, tsst;
		typename swizzler_wrapper<1, 0, 0, 2>::type yxxz, grrb, tssp;
		typename swizzler_wrapper<1, 0, 0, 3>::type yxxw, grra, tssq;
		typename swizzler_wrapper<1, 0, 1, 0>::type yxyx, grgr, tsts;
		typename swizzler_wrapper<1, 0, 1, 1>::type yxyy, grgg, tstt;
		typename swizzler_wrapper<1, 0, 1, 2>::type yxyz, grgb, tstp;
		typename swizzler_wrapper<1, 0, 1, 3>::type yxyw, grga, tstq;
		typename swizzler_wrapper<1, 0, 2, 0>::type yxzx, grbr, tsps;
		typename swizzler_wrapper<1, 0, 2, 1>::type yxzy, grbg, tspt;
		typename swizzler_wrapper<1, 0, 2, 2>::type yxzz, grbb, tspp;
		typename swizzler_wrapper<1, 0, 2, 3>::type yxzw, grba, tspq;
		typename swizzler_wrapper<1, 0, 3, 0>::type yxwx, grar, tsqs;
		typename swizzler_wrapper<1, 0, 3, 1>::type yxwy, grag, tsqt;
		typename swizzler_wrapper<1, 0, 3, 2>::type yxwz, grab, tsqp;
		typename swizzler_wrapper<1, 0, 3, 3>::type yxww, graa, tsqq;
		typename swizzler_wrapper<1, 1, 0, 0>::type yyxx, ggrr, ttss;
		typename swizzler_wrapper<1, 1, 0, 1>::type yyxy, ggrg, ttst;
		typename swizzler_wrapper<1, 1, 0, 2>::type yyxz, ggrb, ttsp;
		typename swizzler_wrapper<1, 1, 0, 3>::type yyxw, ggra, ttsq;
		typename swizzler_wrapper<1, 1, 1, 0>::type yyyx, gggr, ttts;
		typename swizzler_wrapper<1, 1, 1, 1>::type yyyy, gggg, tttt;
		typename swizzler_wrapper<1, 1, 1, 2>::type yyyz, gggb, tttp;
		typename swizzler_wrapper<1, 1, 1, 3>::type yyyw, ggga, tttq;
		typename swizzler_wrapper<1, 1, 2, 0>::type yyzx, ggbr, ttps;
		typename swizzler_wrapper<1, 1, 2, 1>::type yyzy, ggbg, ttpt;
		typename swizzler_wrapper<1, 1, 2, 2>::type yyzz, ggbb, ttpp;
		typename swizzler_wrapper<1, 1, 2, 3>::type yyzw, ggba, ttpq;
		typename swizzler_wrapper<1, 1, 3, 0>::type yywx, ggar, ttqs;
		typename swizzler_wrapper<1, 1, 3, 1>::type yywy, ggag, ttqt;
		typename swizzler_wrapper<1, 1, 3, 2>::type yywz, ggab, ttqp;
		typename swizzler_wrapper<1, 1, 3, 3>::type yyww, ggaa, ttqq;
		typename swizzler_wrapper<1, 2, 0, 0>::type yzxx, gbrr, tpss;
		typename swizzler_wrapper<1, 2, 0, 1>::type yzxy, gbrg, tpst;
		typename swizzler_wrapper<1, 2, 0, 2>::type yzxz, gbrb, tpsp;
		typename swizzler_wrapper<1, 2, 0, 3>::type yzxw, gbra, tpsq;
		typename swizzler_wrapper<1, 2, 1, 0>::type yzyx, gbgr, tpts;
		typename swizzler_wrapper<1, 2, 1, 1>::type yzyy, gbgg, tptt;
		typename swizzler_wrapper<1, 2, 1, 2>::type yzyz, gbgb, tptp;
		typename swizzler_wrapper<1, 2, 1, 3>::type yzyw, gbga, tptq;
		typename swizzler_wrapper<1, 2, 2, 0>::type yzzx, gbbr, tpps;
		typename swizzler_wrapper<1, 2, 2, 1>::type yzzy, gbbg, tppt;
		typename swizzler_wrapper<1, 2, 2, 2>::type yzzz, gbbb, tppp;
		typename swizzler_wrapper<1, 2, 2, 3>::type yzzw, gbba, tppq;
		typename swizzler_wrapper<1, 2, 3, 0>::type yzwx, gbar, tpqs;
		typename swizzler_wrapper<1, 2, 3, 1>::type yzwy, gbag, tpqt;
		typename swizzler_wrapper<1, 2, 3, 2>::type yzwz, gbab, tpqp;
		typename swizzler_wrapper<1, 2, 3, 3>::type yzww, gbaa, tpqq;
		typename swizzler_wrapper<1, 3, 0, 0>::type ywxx, garr, tqss;
		typename swizzler_wrapper<1, 3, 0, 1>::type ywxy, garg, tqst;
		typename swizzler_wrapper<1, 3, 0, 2>::type ywxz, garb, tqsp;
		typename swizzler_wrapper<1, 3, 0, 3>::type ywxw, gara, tqsq;
		typename swizzler_wrapper<1, 3, 1, 0>::type ywyx, gagr, tqts;
		typename swizzler_wrapper<1, 3, 1, 1>::type ywyy, gagg, tqtt;
		typename swizzler_wrapper<1, 3, 1, 2>::type ywyz, gagb, tqtp;
		typename swizzler_wrapper<1, 3, 1, 3>::type ywyw, gaga, tqtq;
		typename swizzler_wrapper<1, 3, 2, 0>::type ywzx, gabr, tqps;
		typename swizzler_wrapper<1, 3, 2, 1>::type ywzy, gabg, tqpt;
		typename swizzler_wrapper<1, 3, 2, 2>::type ywzz, gabb, tqpp;
		typename swizzler_wrapper<1, 3, 2, 3>::type ywzw, gaba, tqpq;
		typename swizzler_wrapper<1, 3, 3, 0>::type ywwx, gaar, tqqs;
		typename swizzler_wrapper<1, 3, 3, 1>::type ywwy, gaag, tqqt;
		typename swizzler_wrapper<1, 3, 3, 2>::type ywwz, gaab, tqqp;
		typename swizzler_wrapper<1, 3, 3, 3>::type ywww, gaaa, tqqq;
		typename swizzler_wrapper<2, 0, 0, 0>::type zxxx, brrr, psss;
		typename swizzler_wrapper<2, 0, 0, 1>::type zxxy, brrg, psst;
		typename swizzler_wrapper<2, 0, 0, 2>::type zxxz, brrb, pssp;
		typename swizzler_wrapper<2, 0, 0, 3>::type zxxw, brra, pssq;
		typename swizzler_wrapper<2, 0, 1, 0>::type zxyx, brgr, psts;
		typename swizzler_wrapper<2, 0, 1, 1>::type zxyy, brgg, pstt;
		typename swizzler_wrapper<2, 0, 1, 2>::type zxyz, brgb, pstp;
		typename swizzler_wrapper<2, 0, 1, 3>::type zxyw, brga, pstq;
		typename swizzler_wrapper<2, 0, 2, 0>::type zxzx, brbr, psps;
		typename swizzler_wrapper<2, 0, 2, 1>::type zxzy, brbg, pspt;
		typename swizzler_wrapper<2, 0, 2, 2>::type zxzz, brbb, pspp;
		typename swizzler_wrapper<2, 0, 2, 3>::type zxzw, brba, pspq;
		typename swizzler_wrapper<2, 0, 3, 0>::type zxwx, brar, psqs;
		typename swizzler_wrapper<2, 0, 3, 1>::type zxwy, brag, psqt;
		typename swizzler_wrapper<2, 0, 3, 2>::type zxwz, brab, psqp;
		typename swizzler_wrapper<2, 0, 3, 3>::type zxww, braa, psqq;
		typename swizzler_wrapper<2, 1, 0, 0>::type zyxx, bgrr, ptss;
		typename swizzler_wrapper<2, 1, 0, 1>::type zyxy, bgrg, ptst;
		typename swizzler_wrapper<2, 1, 0, 2>::type zyxz, bgrb, ptsp;
		typename swizzler_wrapper<2, 1, 0, 3>::type zyxw, bgra, ptsq;
		typename swizzler_wrapper<2, 1, 1, 0>::type zyyx, bggr, ptts;
		typename swizzler_wrapper<2, 1, 1, 1>::type zyyy, bggg, pttt;
		typename swizzler_wrapper<2, 1, 1, 2>::type zyyz, bggb, pttp;
		typename swizzler_wrapper<2, 1, 1, 3>::type zyyw, bgga, pttq;
		typename swizzler_wrapper<2, 1, 2, 0>::type zyzx, bgbr, ptps;
		typename swizzler_wrapper<2, 1, 2, 1>::type zyzy, bgbg, ptpt;
		typename swizzler_wrapper<2, 1, 2, 2>::type zyzz, bgbb, ptpp;
		typename swizzler_wrapper<2, 1, 2, 3>::type zyzw, bgba, ptpq;
		typename swizzler_wrapper<2, 1, 3, 0>::type zywx, bgar, ptqs;
		typename swizzler_wrapper<2, 1, 3, 1>::type zywy, bgag, ptqt;
		typename swizzler_wrapper<2, 1, 3, 2>::type zywz, bgab, ptqp;
		typename swizzler_wrapper<2, 1, 3, 3>::type zyww, bgaa, ptqq;
		typename swizzler_wrapper<2, 2, 0, 0>::type zzxx, bbrr, ppss;
		typename swizzler_wrapper<2, 2, 0, 1>::type zzxy, bbrg, ppst;
		typename swizzler_wrapper<2, 2, 0, 2>::type zzxz, bbrb, ppsp;
		typename swizzler_wrapper<2, 2, 0, 3>::type zzxw, bbra, ppsq;
		typename swizzler_wrapper<2, 2, 1, 0>::type zzyx, bbgr, ppts;
		typename swizzler_wrapper<2, 2, 1, 1>::type zzyy, bbgg, pptt;
		typename swizzler_wrapper<2, 2, 1, 2>::type zzyz, bbgb, pptp;
		typename swizzler_wrapper<2, 2, 1, 3>::type zzyw, bbga, pptq;
		typename swizzler_wrapper<2, 2, 2, 0>::type zzzx, bbbr, ppps;
		typename swizzler_wrapper<2, 2, 2, 1>::type zzzy, bbbg, pppt;
		typename swizzler_wrapper<2, 2, 2, 2>::type zzzz, bbbb, pppp;
		typename swizzler_wrapper<2, 2, 2, 3>::type zzzw, bbba, pppq;
		typename swizzler_wrapper<2, 2, 3, 0>::type zzwx, bbar, ppqs;
		typename swizzler_wrapper<2, 2, 3, 1>::type zzwy, bbag, ppqt;
		typename swizzler_wrapper<2, 2, 3, 2>::type zzwz, bbab, ppqp;
		typename swizzler_wrapper<2, 2, 3, 3>::type zzww, bbaa, ppqq;
		typename swizzler_wrapper<2, 3, 0, 0>::type zwxx, barr, pqss;
		typename swizzler_wrapper<2, 3, 0, 1>::type zwxy, barg, pqst;
		typename swizzler_wrapper<2, 3, 0, 2>::type zwxz, barb, pqsp;
		typename swizzler_wrapper<2, 3, 0, 3>::type zwxw, bara, pqsq;
		typename swizzler_wrapper<2, 3, 1, 0>::type zwyx, bagr, pqts;
		typename swizzler_wrapper<2, 3, 1, 1>::type zwyy, bagg, pqtt;
		typename swizzler_wrapper<2, 3, 1, 2>::type zwyz, bagb, pqtp;
		typename swizzler_wrapper<2, 3, 1, 3>::type zwyw, baga, pqtq;
		typename swizzler_wrapper<2, 3, 2, 0>::type zwzx, babr, pqps;
		typename swizzler_wrapper<2, 3, 2, 1>::type zwzy, babg, pqpt;
		typename swizzler_wrapper<2, 3, 2, 2>::type zwzz, babb, pqpp;
		typename swizzler_wrapper<2, 3, 2, 3>::type zwzw, baba, pqpq;
		typename swizzler_wrapper<2, 3, 3, 0>::type zwwx, baar, pqqs;
		typename swizzler_wrapper<2, 3, 3, 1>::type zwwy, baag, pqqt;
		typename swizzler_wrapper<2, 3, 3, 2>::type zwwz, baab, pqqp;
		typename swizzler_wrapper<2, 3, 3, 3>::type zwww, baaa, pqqq;
		typename swizzler_wrapper<3, 0, 0, 0>::type wxxx, arrr, qsss;
		typename swizzler_wrapper<3, 0, 0, 1>::type wxxy, arrg, qsst;
		typename swizzler_wrapper<3, 0, 0, 2>::type wxxz, arrb, qssp;
		typename swizzler_wrapper<3, 0, 0, 3>::type wxxw, arra, qssq;
		typename swizzler_wrapper<3, 0, 1, 0>::type wxyx, argr, qsts;
		typename swizzler_wrapper<3, 0, 1, 1>::type wxyy, argg, qstt;
		typename swizzler_wrapper<3, 0, 1, 2>::type wxyz, argb, qstp;
		typename swizzler_wrapper<3, 0, 1, 3>::type wxyw, arga, qstq;
		typename swizzler_wrapper<3, 0, 2, 0>::type wxzx, arbr, qsps;
		typename swizzler_wrapper<3, 0, 2, 1>::type wxzy, arbg, qspt;
		typename swizzler_wrapper<3, 0, 2, 2>::type wxzz, arbb, qspp;
		typename swizzler_wrapper<3, 0, 2, 3>::type wxzw, arba, qspq;
		typename swizzler_wrapper<3, 0, 3, 0>::type wxwx, arar, qsqs;
		typename swizzler_wrapper<3, 0, 3, 1>::type wxwy, arag, qsqt;
		typename swizzler_wrapper<3, 0, 3, 2>::type wxwz, arab, qsqp;
		typename swizzler_wrapper<3, 0, 3, 3>::type wxww, araa, qsqq;
		typename swizzler_wrapper<3, 1, 0, 0>::type wyxx, agrr, qtss;
		typename swizzler_wrapper<3, 1, 0, 1>::type wyxy, agrg, qtst;
		typename swizzler_wrapper<3, 1, 0, 2>::type wyxz, agrb, qtsp;
		typename swizzler_wrapper<3, 1, 0, 3>::type wyxw, agra, qtsq;
		typename swizzler_wrapper<3, 1, 1, 0>::type wyyx, aggr, qtts;
		typename swizzler_wrapper<3, 1, 1, 1>::type wyyy, aggg, qttt;
		typename swizzler_wrapper<3, 1, 1, 2>::type wyyz, aggb, qttp;
		typename swizzler_wrapper<3, 1, 1, 3>::type wyyw, agga, qttq;
		typename swizzler_wrapper<3, 1, 2, 0>::type wyzx, agbr, qtps;
		typename swizzler_wrapper<3, 1, 2, 1>::type wyzy, agbg, qtpt;
		typename swizzler_wrapper<3, 1, 2, 2>::type wyzz, agbb, qtpp;
		typename swizzler_wrapper<3, 1, 2, 3>::type wyzw, agba, qtpq;
		typename swizzler_wrapper<3, 1, 3, 0>::type wywx, agar, qtqs;
		typename swizzler_wrapper<3, 1, 3, 1>::type wywy, agag, qtqt;
		typename swizzler_wrapper<3, 1, 3, 2>::type wywz, agab, qtqp;
		typename swizzler_wrapper<3, 1, 3, 3>::type wyww, agaa, qtqq;
		typename swizzler_wrapper<3, 2, 0, 0>::type wzxx, abrr, qpss;
		typename swizzler_wrapper<3, 2, 0, 1>::type wzxy, abrg, qpst;
		typename swizzler_wrapper<3, 2, 0, 2>::type wzxz, abrb, qpsp;
		typename swizzler_wrapper<3, 2, 0, 3>::type wzxw, abra, qpsq;
		typename swizzler_wrapper<3, 2, 1, 0>::type wzyx, abgr, qpts;
		typename swizzler_wrapper<3, 2, 1, 1>::type wzyy, abgg, qptt;
		typename swizzler_wrapper<3, 2, 1, 2>::type wzyz, abgb, qptp;
		typename swizzler_wrapper<3, 2, 1, 3>::type wzyw, abga, qptq;
		typename swizzler_wrapper<3, 2, 2, 0>::type wzzx, abbr, qpps;
		typename swizzler_wrapper<3, 2, 2, 1>::type wzzy, abbg, qppt;
		typename swizzler_wrapper<3, 2, 2, 2>::type wzzz, abbb, qppp;
		typename swizzler_wrapper<3, 2, 2, 3>::type wzzw, abba, qppq;
		typename swizzler_wrapper<3, 2, 3, 0>::type wzwx, abar, qpqs;
		typename swizzler_wrapper<3, 2, 3, 1>::type wzwy, abag, qpqt;
		typename swizzler_wrapper<3, 2, 3, 2>::type wzwz, abab, qpqp;
		typename swizzler_wrapper<3, 2, 3, 3>::type wzww, abaa, qpqq;
		typename swizzler_wrapper<3, 3, 0, 0>::type wwxx, aarr, qqss;
		typename swizzler_wrapper<3, 3, 0, 1>::type wwxy, aarg, qqst;
		typename swizzler_wrapper<3, 3, 0, 2>::type wwxz, aarb, qqsp;
		typename swizzler_wrapper<3, 3, 0, 3>::type wwxw, aara, qqsq;
		typename swizzler_wrapper<3, 3, 1, 0>::type wwyx, aagr, qqts;
		typename swizzler_wrapper<3, 3, 1, 1>::type wwyy, aagg, qqtt;
		typename swizzler_wrapper<3, 3, 1, 2>::type wwyz, aagb, qqtp;
		typename swizzler_wrapper<3, 3, 1, 3>::type wwyw, aaga, qqtq;
		typename swizzler_wrapper<3, 3, 2, 0>::type wwzx, aabr, qqps;
		typename swizzler_wrapper<3, 3, 2, 1>::type wwzy, aabg, qqpt;
		typename swizzler_wrapper<3, 3, 2, 2>::type wwzz, aabb, qqpp;
		typename swizzler_wrapper<3, 3, 2, 3>::type wwzw, aaba, qqpq;
		typename swizzler_wrapper<3, 3, 3, 0>::type wwwx, aaar, qqqs;
		typename swizzler_wrapper<3, 3, 3, 1>::type wwwy, aaag, qqqt;
		typename swizzler_wrapper<3, 3, 3, 2>::type wwwz, aaab, qqqp;
		typename swizzler_wrapper<3, 3, 3, 3>::type wwww, aaaa, qqqq;
	};
};

} }