/** @file threadpool/singlethreaded/threadpool.h
 *
 * Threadpool for C++11, singlethreaded version of parallel for_each
 *
 *
 * A thread pool interface with single-threaded implementation.
 * Useful for debugging. Interface identical to VirtualThreadPool.
 * This is used as a fallback implementation on single-threaded
 * systems. Also useful during debugging.
 *
 * @copyright	2014 Ruediger Helsch, Ruediger.Helsch@t-online.de
 * @license	All rights reserved. Use however you want. No warranty at all.
 * $Revision: 2.0 $
 * $Date: 2014/05/14 16:56:58 $
 */
#ifndef THREADPOOL_SINGLETHREADED_PARALLEL_FOR_EACH_H
#define THREADPOOL_SINGLETHREADED_PARALLEL_FOR_EACH_H

#include <cstddef>
#include <type_traits>
#include <memory>
#include <utility>
#include <exception>
#include <future>

#include "../impl/threadpool_impl_util.h"

namespace threadpool {

    namespace singlethreaded {

	namespace parallel {





	    /**
	     * Run a function on all objects in a range of iterators.
	     *
	     * @param first
	     *			The start of the range to be
	     *			processed.
	     *
	     * @param last
	     *			One past the end of the range to be
	     *			processed.
	     *
	     * @param fun
	     *			The function to apply to all objects
	     *			in the range.
	     *
	     * @returns Function
	     *			The final value of the function
	     *
	     * Single-threaded version of parallel_for_each. The template
	     * parameters are unused but left for API compatibility with
	     * parallel_for_each.
	     */
	    template<int default_thread_count = 1,
		     std::size_t maxpart = 0,
		     class Iterator, class Last, class Function,
		     class = typename std::enable_if<std::is_same<Iterator,Last>::value ||
						     !std::is_integral<Iterator>::value ||
						     !std::is_integral<Last>::value
						     >::type
		     >
	    typename std::decay<Function>::type
	    for_each(Iterator first, const Last& last, Function&& fun)
	    {
		while (first != last) {
		    fun(*first);
		    ++first;
		}

		return std::forward<Function>(fun);
	    }

	    /**
	     * Run a function with each of a range of integral values.
	     *
	     * @param first
	     *		The start of the range to be processed.
	     *
	     * @param last
	     *		One past the end of the range to be processed.
	     *
	     * @param fun
	     *		The function to call with all numbers in the
	     *		range.
	     *
	     * Single-threaded version of parallel_for_each. The template
	     * parameters are unused but left for API compatibility with
	     * parallel_for_each.
	     */
	    template<int thread_count = 1,
		     std::size_t maxpart = 0,
		     class Iterator, class Last, class Function,
		     class = typename std::enable_if<!std::is_same<Iterator,Last>::value
						     && std::is_integral<Iterator>::value
						     && std::is_integral<Last>::value
						     >::type
		     >
	    typename std::decay<Function>::type
	    for_each(Iterator&& first, const Last& last, Function&& fun)
	    {
		/*
		  We can not use the generic function because the user
		  might specify `first` as 0 which makes type `Iterator'
		  become `int`, and `last` as something of type
		  `std::size_t` not representable in an `int`. This loop
		  would run forever. Just extend type `Iterator`.
		*/
		typedef typename std::common_type<Iterator, Last>::type common_type;
		typedef ThreadPoolImpl::impl::IntegralIterator<common_type> CommonIterator;

		return for_each<thread_count, maxpart
				>(CommonIterator(std::forward<Iterator>(first)),
				  CommonIterator(last),
				  std::forward<Function>(fun));
	    }

	    /**
	     * Run a function on all objects in a container.
	     *
	     * @param container
	     *			The container.
	     *
	     * @param fun
	     *			The function to apply to all objects
	     *			in the container.
	     *
	     * @returns
	     *			The final value of the function
	     *
	     * Single-threaded version of parallel_for_each. The template
	     * parameters are unused but left for API compatibility with
	     * parallel_for_each.
	     */
	    template<int thread_count = 1,
		     std::size_t maxpart = 0,
		     class Container, class Function>
	    typename std::decay<Function>::type
	    for_each(Container&& container, Function&& fun)
	    {
		return for_each<thread_count, maxpart
				>(std::begin(container),
				  std::end(container),
				  std::forward<Function>(fun));
	    }

	    /**
	     * Run a function on all objects in a container.
	     *
	     * @param container
	     *		The container.
	     *
	     * Version for rvalue containers
	     *
	     * @param fun
	     *		The function to apply to all objects in the
	     *		container.
	     * @returns
	     *			The final value of the function
	     */
	    template<int thread_count = 1,
		     std::size_t maxpart = 0,
		     class Container, class Function,
		     class = typename std::enable_if<!std::is_lvalue_reference<Container>::value>::type>
	    typename std::decay<Function>::type
	    for_each(Container&& container, Function&& fun)
	    {
		return for_each<thread_count, maxpart
				>(std::make_move_iterator(std::begin(container)),
				  std::make_move_iterator(std::end(container)),
				  std::forward<Function>(fun));
	    }

	} // End of namespace parallel

    } // End of namespace singlethreaded

} // End of namespace threadpool

#endif // !defined(THREADPOOL_SINGLETHREADED_PARALLEL_FOR_EACH_H)
