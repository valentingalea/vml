/** @file threadpool/parallel_for_each.h
 *
 * Threadpool for C++11, parallel for_each
 *
 * @copyright	2014 Ruediger Helsch, Ruediger.Helsch@t-online.de
 * @license	All rights reserved. Use however you want. No warranty at all.
 * $Revision: 2.0 $
 * $Date: 2014/05/14 16:56:58 $
 */
#ifndef PARALLEL_FOR_EACH_H
#define PARALLEL_FOR_EACH_H

#include <cstddef>
#include <type_traits>
#include <iterator>
#include <mutex>

#include "impl/threadpool_generic.h"
#include "impl/threadpool_impl_util.h"
#include "singlethreaded/parallel_for_each.h"

namespace ThreadPoolImpl {

    namespace impl {

	/*
	  The thread pool for arbitrary functions works fine, and can be
	  used to process elements of a container. But this means queuing
	  a task for each element, with each task executing the same
	  function on another element of the container. Certainly there
	  is the possibility for optimization.

	  Define a queue calling the same function for each object in an
	  iterator range. This means we do not need a true queue but just
	  an object using incremental values of an iterator until the end
	  of the range is reached.
	*/





	/**
	 * Queue calling the function on single objects.
	 *
	 * @relates ForEachThreadPoolImpl
	 *	Conceptually ForEachThreadPoolImpl_Queue is a member
	 *	of class ForEachThreadPoolImpl, but the standard does
	 *	not allow template specialization inside classes. I
	 *	had to move it out of the class.
	 */
	template<class Iterator, class Last, class Function, bool forward_iterator>
	class ForEachThreadPoolImpl_Queue : public GenericThreadPoolQueue {

	protected:

	    Iterator& current;
	    const Last& last;
	    Function& fun;
	    std::mutex mutex; // Make sure threads do not access concurrently
 
	public:

	    ForEachThreadPoolImpl_Queue(Iterator& first,
					const Last& last,
					Function& fun,
					std::size_t /*ignored*/ = 0)
		: current(first),
		  last(last),
		  fun(fun)
	    { }

	    void work(bool /*ignored*/) override
	    {
		const Last& l(last);
		for (;;) {
		    std::unique_lock<std::mutex> lock(mutex);
		    if (current == l)
			break;
		    typename iterval_traits<Iterator>::type v(iterval_traits<Iterator>::copy(current));
		    ++current;
		    lock.unlock();
		    fun(iterval_traits<Iterator>::pass(std::move(v)));
		}
	    }

	    /**
	     * Shut the queue down, stop returning values
	     */
	    void shutdown() override
	    {
		std::lock_guard<std::mutex> lock(mutex);
		current = last;
	    }
	};





	/*
	  The queue just implemented would work fine. But if there are
	  a lot of tasks with each task taking a very short time, it may
	  cause a lot of overhead because each object is dequeued
	  separately. Wouldn't it be nice if we could deliver larger
	  tasks?
	*/

	/**
	 * Run a function on objects from a container.
	 *
	 * Queue with `forward_iterator` == false takes groups of
	 * objects from the queue.
	 *
	 * This works only for random access iterators. The
	 * specialization is selected with template parameter
	 * forward_iterator = true. For all other iterators, use the
	 * general case of the template above.
	 *
	 * @relates ForEachThreadPoolImpl
	 *	Conceptually ForEachThreadPoolImpl_Queue is a member
	 *	of class ForEachThreadPoolImpl, but the standard does
	 *	not allow template specialization inside classes. I
	 *	had to move it out of the class.
	 */
	template<class Iterator, class Last, class Function>
	class ForEachThreadPoolImpl_Queue<Iterator, Last, Function, true>
	    : public ForEachThreadPoolImpl_Queue<Iterator, Last, Function, false> {

	    typedef  ForEachThreadPoolImpl_Queue<Iterator, Last, Function, false> Base;

	    const std::size_t maxpart;
	    typename std::iterator_traits<Iterator>::difference_type remaining;


	public:

	    ForEachThreadPoolImpl_Queue(Iterator& first,
					const Last& last,
					Function& fun,
					std::size_t maxpart)
		: Base(first, last, fun),
		  maxpart(maxpart),
		  remaining(std::distance(first, last))
	    { }

	    void work(bool /*ignored*/) override
	    {
		const Last& last = this->last; // Does never change
		for (;;) {
		    Iterator c, l;
		    {
			std::lock_guard<std::mutex> lock(this->mutex);

			if ((c = this->current) == last)
			    break;

			typename std::iterator_traits<Iterator>::difference_type stride =
			    (maxpart == 0) ? 1 : remaining / maxpart;
			if (stride <= 0)
			    stride = 1;

			l = c;
			std::advance(l, stride);
			this->current = l;
			remaining -= stride;
		    }

		    while (c != l) {
			this->fun(*c);
			++c;
		    }
		}
	    }

	};





	/*
	  Now that the queue implementation is done, the definition of a
	  thread pool for container processing is easy.
	*/

	/**
	 * A thread pool to be used as base for container processing.
	 *
	 * @tparam Iterator
	 *		The iterator type to be used to traverse the container.
	 *
	 * @tparam Last
	 *		The iterator type for the last element.
	 *
	 * @tparam Function
	 *		The function type to execute. Must be callable
	 *		with a reference to a container value_type as
	 *		parameter, e.g.  as void(Element& e).
	 *
	 */
	template<class Iterator, class Last, class Function>
	class ForEachThreadPoolImpl {

	    typedef ForEachThreadPoolImpl_Queue<Iterator, Last, Function,
						std::is_base_of<std::forward_iterator_tag,
								typename std::iterator_traits<Iterator>::iterator_category
								>::value
						> Queue;

	    Queue queue;
	    GenericThreadPool pool;

	public:

	    /**
	     * Run a function on all objects in an iterator range.
	     *
	     * @param first
	     *			The start of the range to be processed.
	     *
	     * @param last
	     *			One past the end of the range to be processed.
	     *
	     * @param fun
	     *			The function to apply to all objects
	     *			in the container.
	     *
	     * @param thread_count
	     *			The number of threads to use. If the
	     *			thread count is specified as -1 it
	     *			defaults to the number of available
	     *			hardware threads
	     *			std::thread::hardware_concurrency().
	     *
	     * @param maxpart
	     *			The maximum part of the remaining
	     *			input range that one thread is allowed
	     *			to take.  If maxpart is for example 5
	     *			and 100 elements remain to be
	     *			processed, then a task will take 100 /
	     *			5 = 20 elements and process them. If a
	     *			large value is chosen for maxpart,
	     *			each thread will take small chunks of
	     *			work and will look for more work
	     *			frequently, causing increased
	     *			synchronization overhead. If a small
	     *			value is chosen for maxpart, each
	     *			thread will take huge chunks of work,
	     *			possibly leaving the remaining threads
	     *			out of work at the end. A good value
	     *			might be three times the number of
	     *			threads. A value of 0 enforces
	     *			single-object processing.
	     */
	    ForEachThreadPoolImpl(Iterator& first, const Last& last,
				  Function& fun,
				  int thread_count,
				  std::size_t maxpart)
		: queue(first, last, fun, maxpart),
		  pool(queue, thread_count)
	    { }

	    /**
	     * Collect threads, throw any pending exceptions.
	     *
	     * Use this function if waiting in the desctructor or throwing
	     * exceptions from the destructor is undesirable.  After
	     * join() returned, the thread pool can be destroyed without
	     * delay and without throwing.
	     */
	    void join()
	    {
		pool.join();
	    }

	};





    } // End of namespace impl

} // End of namespace ThreadPoolImpl



namespace threadpool {

    namespace parallel {

	/**
	 * Run a function on all objects in a range of iterators.
	 *
	 * @param first
	 *		The start of the range to be processed.
	 *
	 * @param last
	 *		One past the end of the range to be processed.
	 *
	 * @param fun
	 *		The function to apply to all objects in the
	 *		range.
	 *
	 * @returns
	 *		The final value of the function
	 *
	 * @tparam thread_count
	 *		The number of threads to spawn. If the default
	 *		value of -1 is specified, the thread count is
	 *		determined based on the number of available
	 *		hardware threads. A value of 1 selects the
	 *		single-threaded algorithm.
	 *
	 * @tparam maxpart The maximum part of the remaining input
	 *		range that one thread is allowed to take.  If
	 *		maxpart is for example 5 and 100 elements
	 *		remain to be processed, then a task will take
	 *		100 / 5 = 20 elements and process them. If a
	 *		large value is chosen for maxpart, each thread
	 *		will take small chunks of work and will look
	 *		for more work frequently, causing increased
	 *		synchronization overhead. If a small value is
	 *		chosen for maxpart, each thread will take huge
	 *		chunks of work, possibly leaving the remaining
	 *		threads out of work at the end. A good value
	 *		might be three times the number of
	 *		threads. The default value of 1 lets the
	 *		system determine a value, which is three times
	 *		the number of threads. A value of 0 enforces
	 *		single-object processing.
	 */
	template<int thread_count = -1,
		 std::size_t maxpart = 1,
		 class Iterator, class Last, class Function,
		 class = typename std::enable_if<std::is_same<Iterator,Last>::value ||
						 !std::is_integral<Iterator>::value ||
						 !std::is_integral<Last>::value
						 >::type
		 >
	typename std::decay<Function>::type
	for_each(Iterator first, const Last& last, Function&& fun)
	{

	    unsigned int tc =
		ThreadPoolImpl::impl::GenericThreadPool::determine_thread_count(thread_count);

	    if (tc <= 1) {
		return singlethreaded::parallel::for_each(first, last, fun);
	    } else {
		ThreadPoolImpl::impl::ForEachThreadPoolImpl<Iterator, Last, Function
							    >(first, last, fun, thread_count,
							      maxpart != 1 ? maxpart : 3 * (tc + 1));
		return std::forward<Function>(fun);
	    }
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
	 * @returns
	 *		The final value of the function
	 *
	 * @tparam thread_count
	 *		The number of threads to spawn. If the default
	 *		value of -1 is specified, the thread count is
	 *		determined based on the number of available
	 *		hardware threads. A value of 1 selects the
	 *		single-threaded algorithm.
	 *
	 * @tparam maxpart The maximum part of the remaining input
	 *		range that one thread is allowed to take.  If
	 *		maxpart is for example 5 and 100 elements
	 *		remain to be processed, then a task will take
	 *		100 / 5 = 20 elements and process them. If a
	 *		large value is chosen for maxpart, each thread
	 *		will take small chunks of work and will look
	 *		for more work frequently, causing increased
	 *		synchronization overhead. If a small value is
	 *		chosen for maxpart, each thread will take huge
	 *		chunks of work, possibly leaving the remaining
	 *		threads out of work at the end. A good value
	 *		might be three times the number of
	 *		threads. The default value of 1 lets the
	 *		system determine a value, which is three times
	 *		the number of threads. A value of 0 enforces
	 *		single-object processing.
	 */
	template<int thread_count = -1,
		 std::size_t maxpart = 1,
		 class Iterator, class Last, class Function,
		 class = typename std::enable_if<!std::is_same<Iterator,Last>::value &&
						 std::is_integral<Iterator>::value &&
						 std::is_integral<Last>::value
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
	 * Version for lvalue containers. The objects in the container
	 * are passed to `fun` by reference.
	 *
	 * @param container
	 *		The container.
	 *
	 * @param fun
	 *		The function to apply to all objects in the
	 *		container.
	 *
	 * @returns
	 *		The final value of the function
	 *
	 * @tparam thread_count
	 *		The number of threads to spawn. If the default
	 *		value of -1 is specified, the thread count is
	 *		determined based on the number of available
	 *		hardware threads. A value of 1 selects the
	 *		single-threaded algorithm.
	 *
	 * @tparam maxpart The maximum part of the remaining input
	 *		range that one thread is allowed to take.  If
	 *		maxpart is for example 5 and 100 elements
	 *		remain to be processed, then a task will take
	 *		100 / 5 = 20 elements and process them. If a
	 *		large value is chosen for maxpart, each thread
	 *		will take small chunks of work and will look
	 *		for more work frequently, causing increased
	 *		synchronization overhead. If a small value is
	 *		chosen for maxpart, each thread will take huge
	 *		chunks of work, possibly leaving the remaining
	 *		threads out of work at the end. A good value
	 *		might be three times the number of
	 *		threads. The default value of 1 lets the
	 *		system determine a value, which is three times
	 *		the number of threads. A value of 0 enforces
	 *		single-object processing.
	 */
	template<int thread_count = -1,
		 std::size_t maxpart = 1,
		 class Container, class Function>
	typename std::decay<Function>::type
	for_each(Container& container, Function&& fun)
	{
	    return for_each<thread_count, maxpart
			    >(std::begin(container),
			      std::end(container),
			      std::forward<Function>(fun));
	}

	/**
	 * Run a function on all objects in a container.
	 *
	 * Version for rvalue containers. The objects in the container
	 * are passed to `fun` by rvalue reference, so they can be
	 * move()d.
	 *
	 * @param container
	 *		The container.
	 *
	 * @param fun
	 *		The function to apply to all objects in the
	 *		container.
	 * @returns
	 *		The final value of the function
	 *
	 * @tparam thread_count
	 *		The number of threads to spawn. If the default
	 *		value of -1 is specified, the thread count is
	 *		determined based on the number of available
	 *		hardware threads. A value of 1 selects the
	 *		single-threaded algorithm.
	 *
	 * @tparam maxpart The maximum part of the remaining input
	 *		range that one thread is allowed to take.  If
	 *		maxpart is for example 5 and 100 elements
	 *		remain to be processed, then a task will take
	 *		100 / 5 = 20 elements and process them. If a
	 *		large value is chosen for maxpart, each thread
	 *		will take small chunks of work and will look
	 *		for more work frequently, causing increased
	 *		synchronization overhead. If a small value is
	 *		chosen for maxpart, each thread will take huge
	 *		chunks of work, possibly leaving the remaining
	 *		threads out of work at the end. A good value
	 *		might be three times the number of
	 *		threads. The default value of 1 lets the
	 *		system determine a value, which is three times
	 *		the number of threads. A value of 0 enforces
	 *		single-object processing.
	 */
	template<int thread_count = -1,
		 std::size_t maxpart = 1,
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

} // End of namespace threadpool

#endif // !defined(PARALLEL_FOR_EACH_H)
