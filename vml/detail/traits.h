#pragma once

namespace vml { namespace traits {

// get the common type between 2 vectors (including scalar promotions aka vec1's)
// rules:
//   - their underlying scalar types must have same common type
//   - if vector of different dimensions, at least one must be vec1 (promoted scalar)
// output:
//   the higher dimension vector is chosen
template<
	class vector_type_1, class scalar_type_1, size_t size_1,
	class vector_type_2, class scalar_type_2, size_t size_2
>
struct common_vec_type_impl
{
	using scalar_common_type = typename std::common_type<scalar_type_1, scalar_type_2>::type;
	static_assert(std::is_same<scalar_common_type, scalar_type_1>::value ||
		std::is_same<scalar_common_type, scalar_type_2>::value,
		"invalid vector common scalar type");
	static_assert(size_1 == size_2 || size_1 == 1 || size_2 == 1,
		"vector sizes must be equal or at least one needs to be a promoted scalar");

	using type = typename std::conditional<
		// if same dimensions, choose the one with the common scalar
		size_1 == size_2,
		typename std::conditional<std::is_same<scalar_common_type, scalar_type_1>::value,
		vector_type_1, vector_type_2>::type,
		// else (diff dimensions) choose the bigger one
		typename std::conditional<size_1 == 1,
		vector_type_2, vector_type_1>::type
	>::type;
};

template<class V1, class V2>
struct common_vec_type :
	common_vec_type_impl<
	V1, typename V1::scalar_type, V1::num_components,
	V2, typename V2::scalar_type, V2::num_components
	>
{};

// get the equivalent vec type doing necesary promotion (scalar to vec1 for ex)
// this is the general case operating on a list of types
template<class T, class... Ts>
struct promote_to_vec :
	common_vec_type<
	typename promote_to_vec<T>::type,
	typename promote_to_vec<Ts...>::type
	>
{};

// implementation class, will get specialized
template<class T>
struct promote_to_vec_impl
{};

// specialization for promotion of a single type, defers to impl class
template<class T>
struct promote_to_vec<T> :
	promote_to_vec_impl<typename ::vml::detail::remove_cvref<T>::type>
{};

// specialization: vector just returns itself
template<typename T, size_t... Ns>
struct promote_to_vec_impl<::vml::vector<T, Ns...>>
{
	using type = ::vml::vector<T, Ns...>;
};

// specialization: swizzlers return their equivalent vec (.xx -> vec2)
template<typename vector_type, typename T, size_t N, size_t... indices>
struct promote_to_vec_impl<::vml::detail::swizzler<vector_type, T, N, indices...>>
{
	using type = vector_type;
};

// specialization: aritmethic scalars are promoted to vec1
template<typename T>
struct scalar_to_vector
{
	using type = ::vml::vector<T, 0>;
};
template<> struct promote_to_vec_impl<bool> : scalar_to_vector<bool> {};
template<> struct promote_to_vec_impl<int> : scalar_to_vector<int> {};
template<> struct promote_to_vec_impl<long int> : scalar_to_vector<long int> {};
template<> struct promote_to_vec_impl<float> : scalar_to_vector<float> {};
template<> struct promote_to_vec_impl<double> : scalar_to_vector<double> {};

constexpr int vec_traits_test()
{
	using  vec1 = ::vml::vector<float, 0>;
	using dvec1 = ::vml::vector<double, 0>;
	using  vec3 = ::vml::vector<float, 0, 1, 2>;

	static_assert(std::is_convertible<vec1, float>::value, "mismatch");

	static_assert(std::is_same<promote_to_vec<float>::type, vec1>::value, "mismatch");
	static_assert(std::is_same<promote_to_vec<vec3>::type, vec3>::value, "mismatch");
	static_assert(std::is_same<promote_to_vec<decltype(vec3().xyz)>::type, vec3>::value, "mismatch");

	static_assert(std::is_same<promote_to_vec<float, float>::type, vec1>::value, "mismatch");
	static_assert(std::is_same<promote_to_vec<float, double>::type, dvec1>::value, "mismatch");
	static_assert(std::is_same<promote_to_vec<vec3, float>::type, vec3>::value, "mismatch");
	static_assert(std::is_same<promote_to_vec<double, vec3>::type, vec3>::value, "mismatch");
	static_assert(std::is_same<promote_to_vec<vec3, float, double>::type, vec3>::value, "mismatch");

	return 0;
}

static constexpr auto vec_traits_unit_test = vec_traits_test();

} } // vml::traits