/** @file threadpool/singlethreaded/parallel_transform.h
 *
 * Threadpool for C++11, singlethreaded version of parallel transform
 *
 *
 * A thread pool interface with single-threaded implementation.
 * Useful for debugging. Interface identical to multithreaded parallel
 * for_each.  This is used as a fallback implementation on
 * single-threaded systems. Also useful during debugging.
 *
 * @copyright	2014 Ruediger Helsch, Ruediger.Helsch@t-online.de
 * @license	All rights reserved. Use however you want. No warranty at all.
 * $Revision: 2.0 $
 * $Date: 2014/05/14 16:56:58 $
 */
#ifndef THREADPOOL_SINGLETHREADED_PARALLEL_TRANSFORM_H
#define THREADPOOL_SINGLETHREADED_PARALLEL_TRANSFORM_H

#include <cstddef>
#include <type_traits>
#include <memory>
#include <utility>

#include "../impl/threadpool_impl_util.h"

namespace threadpool {

    namespace singlethreaded {

	namespace parallel {





	    /**
	     * Run a function on all objects in a range of iterators,
	     * and store the return values to on output iterator
	     *
	     * @param first
	     *			The start of the range to be
	     *			processed.
	     *
	     * @param last
	     *			One past the end of the range to be
	     *			processed.
	     *
	     * @param result
	     *			One past the end of the range to be
	     *			processed.
	     *
	     * @param fun
	     *			The function to apply to all objects
	     *			in the range.
	     *
	     * @returns
	     *			The iterator past the last result value.
	     *
	     * Single-threaded version of parallel_transform. The template
	     * parameters are unused but left for API compatibility with
	     * parallel_transform.
	     */
	    template<int thread_count = 1,
		     std::size_t maxpart = 1,
		     class InputIterator, class Last, class OutputIterator, class Function,
		     class = typename std::enable_if<std::is_same<InputIterator,Last>::value ||
						     !std::is_integral<InputIterator>::value ||
						     !std::is_integral<Last>::value
						     >::type
		     >
	    typename std::decay<OutputIterator>::type
	    transform(InputIterator first, const Last& last,
		      OutputIterator result, Function&& fun)
	    {
		while (first != last) {
		    *result = fun(*first);
		    ++result;
		    ++first;
		}
		return result;
	    }

	    /**
	     * Run a function with each of a range of integral values, store
	     * the return values through an output iterator.
	     *
	     * @param first
	     *		The start of the range to be processed.
	     *
	     * @param last
	     *		One past the end of the range to be processed.
	     *
	     * @param result
	     *		The iterator receiving the return values
	     *
	     * @param fun
	     *		The function to call with all numbers in the
	     *		range.
	     */
	    template<int thread_count = 1,
		     unsigned int maxpart = 0,
		     class InputIterator, class Last, class OutputIterator, class Function,
		     class = typename std::enable_if<!std::is_same<InputIterator,Last>::value &&
						     std::is_integral<InputIterator>::value &&
						     std::is_integral<Last>::value
						     >::type
		     >
	    typename std::decay<OutputIterator>::type
	    transform(InputIterator&& first, const Last& last,
		      OutputIterator&& result, Function&& fun)
	    {
		/*
		  We can not use the generic function because the user
		  might specify `first` as 0 which makes type
		  `InputIterator' become `int`, and `last` as something of
		  type `std::size_t` not representable in an `int`. This
		  loop would run forever. Just extend type `InputIterator`.
		*/
		typedef typename std::common_type<InputIterator, Last>::type common_type;
		typedef ThreadPoolImpl::impl::IntegralIterator<common_type> CommonIterator;

		return transform<thread_count, maxpart
				 >(CommonIterator(std::forward<InputIterator>(first)),
				   CommonIterator(last),
				   std::forward<OutputIterator>(result),
				   std::forward<Function>(fun));
	    }

	    /**
	     * Run a function on all objects in a container,
	     * store the return values to an output iterator
	     *
	     * Version for lvalue containers
	     *
	     * @param container
	     *			The container with the input values.
	     *
	     * @param result
	     *			The iterator receiving the return values
	     *
	     * @param fun
	     *			The function to apply to all objects
	     *			in the range.
	     *
	     * @returns OutputIterator
	     *			The iterator past the last result value.
	     *
	     * Single-threaded version of parallel_transform. The template
	     * parameters are unused but left for API compatibility with
	     * parallel_transform.
	     */
	    template<int thread_count = 1,
		     std::size_t maxpart = 1,
		     class Container, class OutputIterator, class Function>
	    typename std::decay<OutputIterator>::type
	    transform(Container& container,
		      OutputIterator&& result, Function&& fun)
	    {
		return transform<thread_count, maxpart
				 >(std::begin(container),
				   std::end(container),
				   std::forward<OutputIterator>(result),
				   std::forward<Function>(fun));
	    }


	    /**
	     * Run a function on all objects in a container, store the
	     * return values through an output iterator.
	     *
	     * @param container
	     *		The container.
	     *
	     * @param result
	     *		The iterator receiving the return values
	     *
	     * @param fun
	     *		The function to apply to all objects in the
	     *		container.
	     *
	     * @returns
	     *		The final value of the result iterator
	     */
	    template<int thread_count = 1,
		     std::size_t maxpart = 0,
		     class Container, class OutputIterator, class Function,
		     class = typename std::enable_if<!std::is_lvalue_reference<Container>::value>::type>
	    typename std::decay<OutputIterator>::type
	    transform(Container&& container,
		      OutputIterator&& result, Function&& fun)
	    {
		return transform<thread_count, maxpart
				 >(std::make_move_iterator(std::begin(container)),
				   std::make_move_iterator(std::end(container)),
				   std::forward<OutputIterator>(result),
				   std::forward<Function>(fun));
	    }

	} // End of namespace parallel

    } // End of namespace singlethreaded

} // End of namespace threadpool

#endif // !defined(THREADPOOL_SINGLETHREADED_PARALLEL_TRANSFORM_H)
