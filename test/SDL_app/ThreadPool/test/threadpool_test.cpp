/** @file threadpool_test.cpp
 *
 * Test suite for thread pool
 *
 * @copyright	2014 Ruediger Helsch, Ruediger.Helsch@t-online.de
 * @license	All rights reserved. Use however you want. No warranty at all.
 * $Revision: 2.1 $
 * $Date: 2014/05/15 23:55:22 $
 */
#define BOOST_TEST_MODULE threadpool_test
#include <vector>
#include <list>
#include <ostream>
#include <iostream>
#include <chrono>
#include <cmath>
#include <iterator>
#define BOOST_TEST_MAIN





namespace boost {

namespace test_tools {

    template<typename T>
    std::ostream& operator<<(std::ostream& s, const std::vector<T>& r) {
	int preceding = 0;
	s << "{";
	for (const T& p: r) {
	    if (preceding++) s << ",";
	    s << p;
	}
	s << "}";
	return s;
    }

}}

#define CONTAINER_CHECK_EQUAL(a, b) \
    { \
	auto btmp = std::vector<int>b; \
	BOOST_CHECK_EQUAL(std::distance(std::begin(a), std::end(a)), std::distance(std::begin(btmp), std::end(btmp))); \
	auto ap = std::begin(a); \
	auto ae = std::end(a); \
	auto bp = std::begin(btmp); \
	while (ap != ae) { \
	    BOOST_CHECK_EQUAL(*ap, *bp); \
	    ++ap; ++bp; \
	} \
    }



#include <boost/test/included/unit_test.hpp>
using boost::unit_test::test_suite;





#include "../include/threadpool/threadpool.h"
#include "../include/threadpool/parallel_for_each.h"
#include "../include/threadpool/parallel_transform.h"
#include "../include/threadpool/singlethreaded/threadpool.h"
#include "../include/threadpool/singlethreaded/parallel_for_each.h"
#include "../include/threadpool/singlethreaded/parallel_transform.h"
#include "../include/threadpool/make_iterator.h"

#include "../include/threadpool/impl/threadpool_impl_homogenous.h"






namespace threadpool_test {





    // Name of the current test case for diagnostic messages
#define TEST_CASE_NAME boost::unit_test::framework::current_test_case().p_name

    // Sink values so computations are not optimized away
    volatile double sink;





    /*
      Functions for timing
    */
    typedef std::chrono::steady_clock Clock;
    Clock::time_point now() {
	return Clock::now();
    }
    double duration_in_microseconds(const Clock::time_point& t0,
				    const Clock::time_point& t1) {
	return static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(t1-t0).count());
    }





    static auto lfun = [](int&e){ e *= 2; };
    //static auto lfun = [](int&e){ e *= 2; for (int i=0;i<100;++i)sink+=std::cos(std::sin((double)i*e*sink));};
    static auto tfun = [](int e) -> int { lfun(e); return e; };

    /*
      Write test cases as function templates so they can be used on different
      thread pool implementations.
    */
    template<class Pool>
    void for_each_static_member_Tests() {
	{ // Multithreaded for_each, std::vector
	    std::vector<int> a = {0,1,2,3,4,5,6,7,8,9};
	    Pool::for_each(a, [](int&e){ e *= 2; });
	    const std::vector<int> b({0,2,4,6,8,10,12,14,16,18});
	    BOOST_CHECK_EQUAL(a, std::vector<int>({0,2,4,6,8,10,12,14,16,18}));
	}
	{ // Multithreaded for_each, array
	    int a[] = {0,1,2,3,4,5,6,7,8,9};
	    Pool::for_each(a, lfun);
	    int b[] = {0,2,4,6,8,10,12,14,16,18};
	    //BOOST_CHECK_EQUAL(a, b);
	    for (unsigned int i = 0; i < sizeof a / sizeof a[0]; ++i)
		BOOST_CHECK_EQUAL(a[i], b[i]);
	}
	{ // Large number of short tasks
	    unsigned int size = 1000000;
	    std::vector<int> a(size);
	    auto t0 = now();
	    for (unsigned int i = 0; i < a.size(); ++i)
		a[i] = i;
	    auto t1 = now();
	    Pool::for_each(a, lfun);
	    auto t2 = now();
	    double d1 = duration_in_microseconds(t0, t1) / size;
	    double d2 = duration_in_microseconds(t1, t2) / size;
	    std::cout << TEST_CASE_NAME << ": run() = " << d1 << " us, for_each() = " << d2 << " us" << std::endl;
	    for (unsigned int i = 0; i < a.size(); ++i)
		BOOST_CHECK_EQUAL(a[i], 2*i);
	}
    }





    /*
      One test case for each feature.
    */
    void for_each_tests_fun1(int&e){ e *= 2; }
    template<class C>
    void for_each_tests()
    {
	{ // Multithreaded for_each, std::vector
	    std::vector<int> a({0,1,2,3,4,5,6,7,8,9});
	    C::parallel_for_each(a, lfun);
	    BOOST_CHECK_EQUAL(a, std::vector<int>({0,2,4,6,8,10,12,14,16,18}));
	}
	{ // Multithreaded for_each, std::vector&
	    std::vector<int> a1({0,1,2,3,4,5,6,7,8,9});
	    std::vector<int>& a = a1;
	    C::parallel_for_each(a, lfun);
	    BOOST_CHECK_EQUAL(a, std::vector<int>({0,2,4,6,8,10,12,14,16,18}));
	}
	{ // Multithreaded for_each, array
	    int a[] = {0,1,2,3,4,5,6,7,8,9};
	    C::parallel_for_each(a, lfun);
	    CONTAINER_CHECK_EQUAL(a, ({0,2,4,6,8,10,12,14,16,18}));
	}
	{ // Multithreaded for_each, list
	    std::list<int> a = {0,1,2,3,4,5,6,7,8,9};
	    C::parallel_for_each(a, lfun);
	    CONTAINER_CHECK_EQUAL(a, ({0,2,4,6,8,10,12,14,16,18}));
	}
	{ // Multithreaded for_each, iterator
	    std::vector<int> a = {0,1,2,3,4,5,6,7,8,9};
	    C::parallel_for_each(a.begin(), a.end(), lfun);
	    CONTAINER_CHECK_EQUAL(a, ({0,2,4,6,8,10,12,14,16,18}));
	}
	{ // Multithreaded for_each, iterator&
	    std::vector<int> a = {0,1,2,3,4,5,6,7,8,9};
	    auto beg = a.begin();
	    auto end = a.end();
	    C::parallel_for_each(beg, end, lfun);
	    CONTAINER_CHECK_EQUAL(a, ({0,2,4,6,8,10,12,14,16,18}));
	}
	{ // Multithreaded for_each, naked pointer
	    std::vector<int> a = {0,1,2,3,4,5,6,7,8,9};
	    int* beg = &a[0];
	    int* end = beg+a.size();;
	    C::parallel_for_each(beg, end, lfun);
	    CONTAINER_CHECK_EQUAL(a, ({0,2,4,6,8,10,12,14,16,18}));
	}
	{ // Multithreaded for_each, integer range
	    std::vector<int> a = {0,1,2,3,4,5,6,7,8,9};
	    C::parallel_for_each(0, a.size(), [&a](int i){ a[i] *= 2; });
	    CONTAINER_CHECK_EQUAL(a, ({0,2,4,6,8,10,12,14,16,18}));
	}
	{ // Multithreaded for_each, lambda reference
	    std::vector<int> a = {0,1,2,3,4,5,6,7,8,9};
	    auto l = lfun;
	    C::parallel_for_each(a, l);
	    CONTAINER_CHECK_EQUAL(a, ({0,2,4,6,8,10,12,14,16,18}));
	}
	{ // Multithreaded for_each, const lambda reference
	    std::vector<int> a = {0,1,2,3,4,5,6,7,8,9};
	    const auto l = lfun;
	    C::parallel_for_each(a, l);
	    CONTAINER_CHECK_EQUAL(a, ({0,2,4,6,8,10,12,14,16,18}));
	}
	{ // Multithreaded for_each, function
	    std::vector<int> a = {0,1,2,3,4,5,6,7,8,9};
	    C::parallel_for_each(a, for_each_tests_fun1);
	    const std::vector<int> b({0,2,4,6,8,10,12,14,16,18});
	    CONTAINER_CHECK_EQUAL(a, ({0,2,4,6,8,10,12,14,16,18}));
	}
	{ // Multithreaded for_each, function*
	    std::vector<int> a = {0,1,2,3,4,5,6,7,8,9};
	    C::parallel_for_each(a, &for_each_tests_fun1);
	    CONTAINER_CHECK_EQUAL(a, ({0,2,4,6,8,10,12,14,16,18}));
	}
	{ // Multithreaded for_each, std::function
	    std::vector<int> a = {0,1,2,3,4,5,6,7,8,9};
	    C::parallel_for_each(a, std::function<void(int&)>(lfun));
	    CONTAINER_CHECK_EQUAL(a, ({0,2,4,6,8,10,12,14,16,18}));
	}
	{ // Multithreaded for_each, std::function&
	    std::vector<int> a = {0,1,2,3,4,5,6,7,8,9};
	    std::function<void(int&)> f(lfun);
	    C::parallel_for_each(a, f);
	    CONTAINER_CHECK_EQUAL(a, ({0,2,4,6,8,10,12,14,16,18}));
	}
	{ // Large number of short tasks, automatic stride
	    unsigned int size = 1000000;
	    std::vector<int> a(size);
	    for (unsigned int i = 0; i < a.size(); ++i)
		a[i] = i;
	    auto t1 = now();
	    C::parallel_for_each(a, lfun);
	    auto t2 = now();
	    double d2 = duration_in_microseconds(t1, t2) / size;
	    std::cout << TEST_CASE_NAME << " for_each() = " << d2 << " us" << std::endl;
	    for (unsigned int i = 0; i < a.size(); ++i)
		BOOST_CHECK_EQUAL(a[i], 2*i);
	}
	{ // Large number of short tasks, forced single-step
	    unsigned int size = 1000000;
	    std::vector<int> a(size);
	    for (unsigned int i = 0; i < a.size(); ++i)
		a[i] = i;
	    auto t1 = now();
	    C::parallel_for_each_1(a, lfun);
	    auto t2 = now();
	    double d2 = duration_in_microseconds(t1, t2) / size;
	    std::cout << TEST_CASE_NAME << " single-step for_each() = " << d2 << " us" << std::endl;
	    for (unsigned int i = 0; i < a.size(); ++i)
		BOOST_CHECK_EQUAL(a[i], 2*i);
	}
    }

    int transform_tests_fun1(const int& e){ return e * 2; }
    template<class C>
    void transform_tests()
    {
	{ // Multithreaded transform, std::vector
	    std::vector<int> a = {0,1,2,3,4,5,6,7,8,9};
	    C::parallel_transform(a, a.begin(), transform_tests_fun1);
	    CONTAINER_CHECK_EQUAL(a, ({0,2,4,6,8,10,12,14,16,18}));
	}
	{ // Multithreaded transform, array
	    int a[] = {0,1,2,3,4,5,6,7,8,9};
	    int r[sizeof a / sizeof a[0]];
	    C::parallel_transform(a, &r[0], transform_tests_fun1);
	    CONTAINER_CHECK_EQUAL(r, ({0,2,4,6,8,10,12,14,16,18}));
	}
	{ // Multithreaded transform, list
	    std::list<int> a = {0,1,2,3,4,5,6,7,8,9};
	    std::list<int> r(a.size());
	    C::parallel_transform(a, r.begin(), transform_tests_fun1);
	    CONTAINER_CHECK_EQUAL(r, ({0,2,4,6,8,10,12,14,16,18}));
	}
	{ // Multithreaded transform, iterator
	    std::vector<int> a = {0,1,2,3,4,5,6,7,8,9};
	    std::vector<int> r(a.size());
	    C::parallel_transform(a.begin(), a.end(), r.begin(), transform_tests_fun1);
	    CONTAINER_CHECK_EQUAL(r, ({0,2,4,6,8,10,12,14,16,18}));
	}
	{ // Multithreaded transform, iterator&
	    std::vector<int> a = {0,1,2,3,4,5,6,7,8,9};
	    std::vector<int> r(a.size());
	    auto beg = a.begin();
	    auto end = a.end();
	    auto res = r.begin();
	    C::parallel_transform(beg, end, res, transform_tests_fun1);
	    CONTAINER_CHECK_EQUAL(r, ({0,2,4,6,8,10,12,14,16,18}));
	}
	{ // Multithreaded transform, naked pointer
	    std::vector<int> a = {0,1,2,3,4,5,6,7,8,9};
	    std::vector<int> r(a.size());
	    int* beg = &a[0];
	    int* end = beg+a.size();;
	    auto res = &r[0];
	    C::parallel_transform(beg, end, res, transform_tests_fun1);
	    CONTAINER_CHECK_EQUAL(r, ({0,2,4,6,8,10,12,14,16,18}));
	}
	{ // Multithreaded transform, lambda
	    std::vector<int> a = {0,1,2,3,4,5,6,7,8,9};
	    C::parallel_transform(a, a.begin(), [](int e) -> int { return 2*e; });
	    CONTAINER_CHECK_EQUAL(a, ({0,2,4,6,8,10,12,14,16,18}));
	}
	{ // Multithreaded transform, move iterator
	    std::vector<int> a({0,1,2,3,4,5,6,7,8,9});
	    std::vector<int> r(10);
	    C::parallel_transform(std::make_move_iterator(std::begin(a)),
				  std::make_move_iterator(std::end(a)),
				  r.begin(),
				  [](const int &e) -> int { return 2*e; });
	    CONTAINER_CHECK_EQUAL(r, ({0,2,4,6,8,10,12,14,16,18}));
	}
	{ // Multithreaded transform, rvalue std::vector
	    std::vector<int> r(10);
	    C::parallel_transform(std::vector<int>({0,1,2,3,4,5,6,7,8,9}), r.begin(),
				  [](const int &e) -> int { return 2*e; });
	    CONTAINER_CHECK_EQUAL(r, ({0,2,4,6,8,10,12,14,16,18}));
	}
	{ // Multithreaded transform, check that input is moved
	    struct A {
		int v;
		A() : v(-1) { };
		A(int v) : v(v) { };
		A(A&& o) : v(o.v) { o.v = -2; }
	    };
	    std::vector<A> a(10);
	    for (unsigned int i = 0; i < a.size(); ++i)
		a[i].v = i;
	    std::vector<int> r;
	    C::parallel_transform(std::move(a), back_inserter(r),
				  [](A&& o) -> int { A a(std::move(o)); return 2 * a.v; });
	    CONTAINER_CHECK_EQUAL(r, ({0,2,4,6,8,10,12,14,16,18}));
	    BOOST_CHECK_EQUAL(a.size(), 10);
	    for (unsigned int i = 0, s = a.size(); i < s; ++i)
		BOOST_CHECK_EQUAL(a[i].v, -2);
	}
	{ // Multithreaded transform, check work with function iterators
	    std::vector<int> a;
	    for (int i: threadpool::make_function_input_range([]() -> int {
			static int u = 1;
			if (u == 10)
			    throw std::out_of_range("");
			return u++;
		    }))
		a.push_back(i);
	    CONTAINER_CHECK_EQUAL(a, ({1,2,3,4,5,6,7,8,9}));
	    std::list<int> b;
	    C::parallel_transform(
				  // Input iterator reads values from array a
				  threadpool::make_function_input_range([&a]() -> int {
					  static unsigned int u = 0;
					  if (u == a.size())
					      throw std::out_of_range("a");
					  return a[u++];
				      }),
				  // Output iterator pushes doubled values to array b
				  threadpool::make_function_output_iterator([&b](int i) {
					  b.push_back(2 * i);
				      }),
				  // Computation function multiplies by 3
				  [](int i) -> int {
				      return 3 * i;
				  });
	    CONTAINER_CHECK_EQUAL(b, ({6,12,18,24,30,36,42,48,54}));
	}
	{ // Multithreaded transform, iterators from l-value references
	    std::vector<int> a;
	    for (int i: threadpool::make_function_input_range([]() -> int {
			static int u = 1;
			if (u == 10)
			    throw std::out_of_range("");
			return u++;
		    }))
		a.push_back(i);
	    CONTAINER_CHECK_EQUAL(a, ({1,2,3,4,5,6,7,8,9}));
	    std::list<int> b;

	    auto ifun = [&a]() -> int {
		static unsigned int u = 0;
		if (u == a.size())
		    throw std::out_of_range("a");
		return a[u++];
	    };

	    auto ofun = [&b](int i) {
		b.push_back(2 * i);
	    };

	    auto tfun = [](int i) -> int {
		return 3 * i;
	    };

	    C::parallel_transform(
				  // Input iterator reads values from array a
				  threadpool::make_function_input_range(ifun),
				  // Output iterator pushes doubled values to array b
				  threadpool::make_function_output_iterator(ofun),
				  // Computation function multiplies by 3
				  tfun);
	    CONTAINER_CHECK_EQUAL(b, ({6,12,18,24,30,36,42,48,54}));
	}
	{ // Timing for random access iterators
	    unsigned int size = 1000000;
	    std::vector<int> a(size);
	    for (unsigned int i = 0; i < a.size(); ++i)
		a[i] = i;
	    std::vector<int> r(size);
	    auto t1 = now();
	    C::parallel_transform(a, r.begin(), tfun);
	    auto t2 = now();
	    double d2 = duration_in_microseconds(t1, t2) / size;
	    std::cout << TEST_CASE_NAME << " random access transform() = " << d2 << " us" << std::endl;
	    for (unsigned int i = 0; i < r.size(); ++i)
		BOOST_CHECK_EQUAL(r[i], 2*i);
	}
	{ // Timing for back_inserter into a list
	    unsigned int size = 1000000;
	    std::vector<int> a(size);
	    for (unsigned int i = 0; i < a.size(); ++i)
		a[i] = i;
	    std::list<int> r;
	    auto t1 = now();
	    C::parallel_transform(a, back_inserter(r), tfun);
	    auto t2 = now();
	    double d2 = duration_in_microseconds(t1, t2) / size;
	    std::cout << TEST_CASE_NAME << " list back_inserter transform() = " << d2 << " us" << std::endl;
	    BOOST_CHECK_EQUAL(r.size(), a.size());
	    auto ri = r.begin();
	    for (unsigned int i = 0, s = r.size(); i < s; ++i)
		BOOST_CHECK_EQUAL(*ri++, 2*i);
	}
    }

    template<class Pool>
    void homogenousFunctionTests() {
	{ // Void lambda function modifying a captured value
	    Pool pool;
	    volatile int i = 1;
	    pool.run([&i](){i *= 3;});
	    pool.wait();
	    BOOST_CHECK_EQUAL(i,3);
	}
	{ // Running function constructed with std::bind()
	    Pool pool;
	    int i = 1;
	    pool.run(std::bind(lfun,std::ref(i)));
	    pool.wait();
	    BOOST_CHECK_EQUAL(i,2);
	}
	{
	    unsigned int size = 1000000;
	    std::vector<int> a(size);
	    for (unsigned int i = 0; i < a.size(); ++i)
		a[i] = i;
	    Pool pool;
	    auto t1 = now();
	    for (auto& e: a)
		pool.run([&e](){e *= 2;});
	    pool.wait();
	    auto t2 = now();
	    double d2 = duration_in_microseconds(t1, t2) / size;
	    std::cout << TEST_CASE_NAME << " run() = " << d2 << " us" << std::endl;
	    for (unsigned int i = 0; i < size; ++i)
		BOOST_CHECK_EQUAL(a[i], 2*i);
	}
	{
	    unsigned int size = 1000000;
	    std::vector<int> a(size);
	    for (unsigned int i = 0; i < a.size(); ++i)
		a[i] = i;
	    Pool pool;
	    auto t1 = now();
	    for (unsigned int i = 0; i < a.size(); ++i)
		pool.run(std::bind(lfun,std::ref(a[i])));
	    pool.wait();
	    auto t2 = now();
	    double d2 = duration_in_microseconds(t1, t2) / size;
	    std::cout << TEST_CASE_NAME << " run(bind()) = " << d2 << " us" << std::endl;
	    for (unsigned int i = 0; i < size; ++i)
		BOOST_CHECK_EQUAL(a[i], 2*i);
	}
	//for_each_static_member_Tests<Pool>();
    }



    int FunctionTests_Helper(int&i) { return i*=2; }

    template<class Pool>
    void FunctionTests() {

	homogenousFunctionTests<Pool>();

	{ // Lambda function returning a result via a future
	    Pool pool;
	    int i = 1;
	    auto f = pool.run([&i]()->int{return i *= 3;});
	    BOOST_CHECK_EQUAL(f.get(),3);
	    BOOST_CHECK_EQUAL(i,3);
	}
	{
	    unsigned int size = 1000000;
	    std::vector<int> a(size);
	    for (unsigned int i = 0; i < a.size(); ++i)
		a[i] = i;
	    std::vector<std::future<int> > f(size);
	    Pool pool;
	    auto t1 = now();
	    for (unsigned int i = 0; i < a.size(); ++i)
		f[i] = pool.run([&a,i]() ->int {return 2*a[i];});
	    pool.wait();
	    auto t2 = now();
	    double d2 = duration_in_microseconds(t1, t2) / size;
	    std::cout << TEST_CASE_NAME << " future = run(lambda) = " << d2 << " us" << std::endl;
	    for (unsigned int i = 0; i < size; ++i)
		BOOST_CHECK_EQUAL(f[i].get(), 2*i);
	}
	{ // std::packaged_task returning a result via a future
	    Pool pool;
	    int i = 1;
	    auto b = std::bind(FunctionTests_Helper, std::ref(i));
	    auto t = std::packaged_task<int()>(std::move(b));
	    auto f = t.get_future();
	    pool.run(std::move(t));
	    BOOST_CHECK_EQUAL(f.get(),2);
	    BOOST_CHECK_EQUAL(i,2);
	}
	{
	    unsigned int size = 1000000;
	    std::vector<int> a(size);
	    for (unsigned int i = 0; i < a.size(); ++i)
		a[i] = i;
	    std::vector<std::future<int> > f(size);
	    Pool pool;
	    auto t1 = now();
	    for (unsigned int i = 0; i < a.size(); ++i) {
		auto t = std::packaged_task<int()>([&a,i]() ->int {return 2*a[i];});
		f[i] = t.get_future();
		pool.run(std::move(t));
	    }
	    pool.wait();
	    auto t2 = now();
	    for (unsigned int i = 0; i < size; ++i)
		BOOST_CHECK_EQUAL(f[i].get(), 2*i);
	    auto t3 = now();
	    double d2 = duration_in_microseconds(t1, t2) / size;
	    double d3 = duration_in_microseconds(t2, t3) / size;
	    std::cout << TEST_CASE_NAME << " future = run(packaged_task) = " << d2 << " us, future.get() = " << d3 << " us"<< std::endl;
	}
	//for_each_static_member_Tests<Pool>();
    }





    BOOST_AUTO_TEST_CASE(debug_test) { // Insert here any debug tests to run first.
    }





    BOOST_AUTO_TEST_CASE(invalid_semantic_Tests) {
	// These should not compile if commented out
	// { typedef threadpool::threadpool Pool; Pool pool; pool.run([](int& i){i+=2;}); }
	// { typedef threadpool::threadpool Pool; Pool pool1; Pool pool2 = pool1; }
    }


    struct parallel_for_each_tests_helper {
	template<class A, class B>
	static void parallel_for_each(A&&a,B&&b)
	{ threadpool::parallel::for_each(std::forward<A>(a),std::forward<B>(b)); }
	template<class A, class B>
	static void parallel_for_each_1(A&&a,B&&b)
	{ threadpool::parallel::for_each<-1,0>(std::forward<A>(a),std::forward<B>(b)); }
	template<class A, class B, class C>
	static void parallel_for_each(A&&a,B&&b,C&&c)
	{ threadpool::parallel::for_each(std::forward<A>(a),std::forward<B>(b),std::forward<C>(c)); }
    };

    BOOST_AUTO_TEST_CASE(parallel_for_each_tests)
    {
	std::cout << "std::thread::hardware_concurrency() = " << std::thread::hardware_concurrency() << std::endl;
	for_each_tests<parallel_for_each_tests_helper>();
    }

    struct singlethreaded_for_each_tests_helper {
	template<class A, class B>
	static void parallel_for_each(A&&a,B&&b)
	{ threadpool::singlethreaded::parallel::for_each(std::forward<A>(a),std::forward<B>(b)); }
	template<class A, class B>
	static void parallel_for_each_1(A&&a,B&&b)
	{ threadpool::singlethreaded::parallel::for_each<-1,0>(std::forward<A>(a),std::forward<B>(b)); }
	template<class A, class B, class C>
	static void parallel_for_each(A&&a,B&&b,C&&c)
	{ threadpool::singlethreaded::parallel::for_each(std::forward<A>(a),std::forward<B>(b),std::forward<C>(c)); }
    };

    BOOST_AUTO_TEST_CASE(singlethreaded_for_each_tests)
    {
	for_each_tests<singlethreaded_for_each_tests_helper>();
    }

    struct parallel_transform_tests_helper {
	template<class A, class B, class C>
	static typename std::remove_reference<B>::type parallel_transform(A&&a,B&&b,C&&c)
	{ return threadpool::parallel::transform(std::forward<A>(a),std::forward<B>(b),std::forward<C>(c)); }
	template<class A, class B, class C>
	static typename std::remove_reference<B>::type parallel_transform(A&&a,A&&e,B&&b,C&&c)
	{ return threadpool::parallel::transform(std::forward<A>(a),std::forward<A>(e),std::forward<B>(b),std::forward<C>(c)); }
    };

    BOOST_AUTO_TEST_CASE(parallel_transform_tests)
    {
	transform_tests<parallel_transform_tests_helper>();
    }

    struct singlethreaded_transform_tests_helper {
	template<class A, class B, class C>
	static typename std::remove_reference<B>::type parallel_transform(A&&a,B&&b,C&&c)
	{ return threadpool::singlethreaded::parallel::transform(std::forward<A>(a),std::forward<B>(b),std::forward<C>(c)); }
	template<class A, class B, class C>
	static typename std::remove_reference<B>::type parallel_transform(A&&a,A&&e,B&&b,C&&c)
	{ return threadpool::singlethreaded::parallel::transform(std::forward<A>(a),std::forward<A>(e),std::forward<B>(b),std::forward<C>(c)); }
    };

    BOOST_AUTO_TEST_CASE(singlethreaded_transform_tests)
    {
	transform_tests<singlethreaded_transform_tests_helper>();
    }

#ifndef _MSC_VER // Visual C++ does not like the union. I think it is fine.

    BOOST_AUTO_TEST_CASE(impl_homogenous_tests)
    {
	const unsigned int queueSize = 1<<16; // Smaller queue stresses wrap-around
	unsigned int size = 1000000;
	std::vector<int> a(size);
	for (unsigned int i = 0; i < a.size(); ++i)
	    a[i] = i;
	auto funproto = std::bind(lfun, std::ref(a[0]));
	//typedef decltype(funproto) funtype;
	//typedef decltype(bind_wrapper(funproto)) pooltype;
	typedef ThreadPoolImpl::impl::HomogenousThreadPoolTemplate<decltype(funproto), 0, queueSize> Pool;
	Pool pool;
	auto t1 = now();
	for (unsigned int i = 0; i < a.size(); ++i)
	    pool.run(std::bind(lfun, std::ref(a[i])));
	pool.wait();
	auto t2 = now();
	double d2 = duration_in_microseconds(t1, t2) / size;
	std::cout << TEST_CASE_NAME << " run() = " << d2 << " us" << std::endl;
	for (unsigned int i = 0; i < size; ++i)
	    BOOST_CHECK_EQUAL(a[i], 2*i);
    }
#endif // !defined(_MSC_VER)

    BOOST_AUTO_TEST_CASE(impl_homogenous_function_tests)
    {
	typedef ThreadPoolImpl::impl::HomogenousThreadPool<std::function<void()> > Pool;
	homogenousFunctionTests<Pool>();
    }

    BOOST_AUTO_TEST_CASE(ThreadPool_tests)
    {
	//typedef ThreadPoolTemplate<-1, 100> Pool; // Smaller queue stresses wrap-around
	typedef threadpool::ThreadPool Pool;
	FunctionTests<Pool>();
    }

#if 0
    BOOST_AUTO_TEST_CASE(LambdaThreadPool_tests)
    {
	homogenousFunctionTests<LambdaThreadPool>();
	{ // Lambda function returning a result via a future
	    LambdaThreadPool pool;
	    volatile int i = 1;
	    auto f = pool.run([&i]()->int{return i * 3;});
	    BOOST_CHECK_EQUAL(f.get(),3);
	    BOOST_CHECK_EQUAL(i,1);
	}
    }
#endif

    BOOST_AUTO_TEST_CASE(singlethreaded_threadpool_tests)
    {
	typedef threadpool::singlethreaded::ThreadPool Pool;
	FunctionTests<Pool>();
    }

} // End of namespace threadpool_test
