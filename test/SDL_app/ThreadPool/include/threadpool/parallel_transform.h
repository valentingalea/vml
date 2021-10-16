/** @file threadpool/parallel_transform.h
 *
 * Threadpool for C++11, parallel transform
 *
 * @copyright	2014 Ruediger Helsch, Ruediger.Helsch@t-online.de
 * @license	All rights reserved. Use however you want. No warranty at all.
 * $Revision: 2.1 $
 * $Date: 2014/05/16 08:24:36 $
 */
#ifndef PARALLEL_TRANSFORM_H
#define PARALLEL_TRANSFORM_H

#include <cstddef>
#include <type_traits>
#include <iterator>
#include <mutex>
#include <condition_variable>

#include "impl/threadpool_generic.h"
#include "impl/threadpool_impl_util.h"
#include "singlethreaded/parallel_transform.h"

namespace ThreadPoolImpl {

    namespace impl {





	/*
	  Now write a parallel version of std::transform().

	  We could reuse a few parts from the parallel for_each
	  implementation, but this would not save much and create a
	  dependence. Better to keep parallel_transform self-standing.
	*/

	/**
	 * Run a function on objects from a container, store the return
	 * values in a result container.
	 *
	 * General case for arbitrary iterators. This is the difficult
	 * case to implement, because the worker threads must synchronize
	 * after they have done their work, and must make sure they write
	 * the results in the order of the input objects and not in the
	 * order the threads are finished. But this is the thing that
	 * makes parallel_transform interesting for the users. They don't
	 * need to synchronize, the algorithm makes it for them.
	 *
	 * @tparam InputIterator
	 *		Type of the input iterator. In this
	 *		specialization with forward_iterator = false,
	 *		an arbitrary input iterator type.
	 *
	 * @tparam OutputIterator
	 *		Type of the result iterator. In this
	 *		specialization with forward_iterator = false,
	 *		an arbitrary output iterator type.
	 *
	 * @tparam Function
	 *		Type of the function to be called with
	 *		successive elements from the input
	 *		iterator. The function must return a result
	 *		which is stored through the result iterator.
	 *
	 * @tparam forward_iterator
	 *		A bool selecting the specialization. The
	 *		general case for arbitrary input and output
	 *		iterators which is implemented here is
	 *		selected with forward_iterator = false. The
	 *		specialization for forward iterators follows
	 *		below.
	 *
	 * @relates TransformThreadPoolImpl
	 *	TransformThreadPoolImpl_Queue is conceptually a member
	 *	of class TransformThreadPoolImpl, but the standard
	 *	does not allow template specialization inside
	 *	classes. I had to move it out of the class.
	 */
	template<class InputIterator, class Last,
		 class OutputIterator, class Function, bool forward_iterator>
	class TransformThreadPoolImpl_Queue : public GenericThreadPoolQueue {

	    struct Results {
		typename std::remove_reference<decltype(std::declval<Function&>()(*std::declval<InputIterator>()))>::type result;
		std::unique_ptr<Results> next;
	    };

	    InputIterator& current;
	    const Last& last;
	    OutputIterator& result;
	    Function& fun;
	    bool do_shutdown = false;
	    std::mutex mutex;


	    typedef unsigned long long int counter_type;
	    counter_type input_counter = 1; // Counter of objects got from the queue
	    counter_type output_counter = 1; // Counter of objects written
	    Results* previous_results = nullptr;
	    counter_type max_output_queue_length = 1000; // This should be configurable
	    std::mutex output_mutex;
	    std::condition_variable output_queue;
	    std::size_t output_queue_waiters = 0;
	


	public:

	    TransformThreadPoolImpl_Queue(InputIterator& first,
					  const Last& last,
					  OutputIterator& result,
					  Function& fun,
					  std::size_t)
		: current(first),
		  last(last),
		  result(result),
		  fun(fun)
	    { }

	    void work(bool return_if_idle) override
	    {

		std::unique_ptr<Results> results;
		const Last& last(this->last); // Does never change.
		for (;;) {
		    if (!results)
			results = std::unique_ptr<Results>(new Results);
		    counter_type ctr;
		    Results* prvres;
		    {
			std::unique_lock<std::mutex> lock(mutex);

			if (current == last)
			    break;

			ctr = input_counter;
			prvres = previous_results;
			previous_results = &*results;
			typename iterval_traits<InputIterator>::type v(iterval_traits<InputIterator>::copy(current));
			++current;
			input_counter = ctr + 1;
			lock.unlock();
			results->result = fun(iterval_traits<InputIterator>::pass(std::move(v)));
		    }


		    {
			/*
			  We must store the results in the order they had
			  in the input sequence, not in the order the
			  tasks finish. Just work together: whoever is
			  ready before his predecessor just leaves his
			  work for the predecessor to clean up.
			*/
			std::unique_lock<std::mutex> lock(output_mutex);

			while (ctr - output_counter > max_output_queue_length) {
			    if (do_shutdown)
				return;
			    if (return_if_idle) {
				prvres->next = std::move(results);
				return;
			    }
			    ++output_queue_waiters;
			    output_queue.wait(lock);
			    --output_queue_waiters;
			}
			
			if (output_counter == ctr) {
			    // Predecessor is done, we can store our things.
			    lock.unlock();
			    *result = std::move(results->result);
			    ++result;
			    ++ctr;
			    lock.lock();
			    // Now look whether our successors have left us their work.
			    while (results->next) {
				results = std::move(results->next);
				lock.unlock();
				*result = std::move(results->result);
				++result;
				++ctr;
				lock.lock();
			    }
			    output_counter = ctr;
			    if (output_queue_waiters)
				output_queue.notify_all(); // All because we do not know who is the right one.
			} else {
			    // Predecessor still running, let him clean up.
			    prvres->next = std::move(results);
			}
		    }

		}
	    }

	    /**
	     * Shut the queue down, stop returning values
	     */
	    void shutdown() override {
		std::lock_guard<std::mutex> lock(mutex);
		std::lock_guard<std::mutex> olock(output_mutex);
		current = last;
		do_shutdown = true;
		output_queue.notify_all();
	    }

	};





	/**
	 * Run a function on objects from a container, store the return
	 * values in a result container.
	 *
	 * Specialization for forward iterators. It is used when
	 * template argument forward_iterator is true. For all other
	 * iterators, use the generic version above.
	 *
	 * @tparam InputIterator
	 *		Type of the input iterator. In this
	 *		specialization with forward_iterator = false,
	 *		an arbitrary input iterator type.
	 *
	 * @tparam OutputIterator
	 *		Type of the result iterator. In this
	 *		specialization with forward_iterator = false,
	 *		an arbitrary output iterator type.
	 *
	 * @tparam Function
	 *		Type of the function to be called with
	 *		successive elements from the input
	 *		iterator. The function must return a result
	 *		which is stored through the result iterator.
	 *
	 * @relates TransformThreadPoolImpl
	 *	TransformThreadPoolImpl_Queue is conceptually a member
	 *	of class TransformThreadPoolImpl, but the standard
	 *	does not allow template specialization inside
	 *	classes. I had to move it out of the class.
	 */
	template<class InputIterator, class Last,
		 class OutputIterator, class Function>
	class TransformThreadPoolImpl_Queue<InputIterator, Last,
					    OutputIterator, Function, true>
	    : public GenericThreadPoolQueue {

	    InputIterator& current;
	    const Last& last;
	    OutputIterator& result;
	    Function& fun;
	    std::mutex mutex;
	    const std::size_t maxpart;
	    typename std::iterator_traits<InputIterator>::difference_type remaining;

	public:

	    TransformThreadPoolImpl_Queue(InputIterator& first,
					  const Last& last,
					  OutputIterator& result,
					  Function& fun,
					  std::size_t maxpart)
		: current(first),
		  last(last),
		  result(result),
		  fun(fun),
		  maxpart(maxpart),
		  remaining(std::distance(first, last))
	    { }

	    void work(bool) override
	    {
		const Last& last(this->last); // Does never change
		for (;;) {
		    InputIterator c, l;
		    OutputIterator r;
		    {
			std::lock_guard<std::mutex> lock(mutex);

			if ((c = current) == last)
			    break;

			typename std::iterator_traits<InputIterator>::difference_type stride =
			    (maxpart == 0) ? 1 : remaining / maxpart;
			if (stride <= 0)
			    stride = 1;

			l = c;
			std::advance(l, stride);
			r = result;
			this->current = l;
			std::advance(result, stride);
			remaining -= stride;
		    }

		    while (c != l) {
			*r = fun(*c);
			++c;
			++r;
		    }
		}
	    }

	    /**
	     * Shut the queue down, stop returning values
	     */
	    void shutdown() override {
		std::lock_guard<std::mutex> lock(mutex);
		current = last;
	    }

	};






	/**
	 * A thread pool to be used as base for container processing.
	 *
	 * @tparam InputIterator
	 *		The iterator type to be used to get input values.
	 *
	 * @tparam OutputIterator
	 *		The iterator type to be used to store the results.
	 *
	 * @tparam Function
	 *		Type of the function to be called with
	 *		successive elements from the input
	 *		iterator. The function must return a result
	 *		which is stored through the result iterator.
	 */
	template<class InputIterator, class Last,
		 class OutputIterator, class Function>
	class TransformThreadPoolImpl {

	    typedef TransformThreadPoolImpl_Queue<InputIterator, Last, OutputIterator, Function,
		std::is_base_of<std::forward_iterator_tag,
				typename std::iterator_traits<InputIterator>::iterator_category
				>::value &&
		std::is_base_of<std::forward_iterator_tag,
				typename std::iterator_traits<OutputIterator>::iterator_category
				>::value
	    > Queue;

	    Queue queue;
	    GenericThreadPool pool;

    public:

	    /**
	     * Run a function on all objects in an input iterator range,
	     * store the return values through an output iterator.
	     *
	     * @param first
	     *			The start of the range to be processed.
	     *
	     * @param last
	     *			One past the end of the range to be processed.
	     *
	     * @param result
	     *			The iterator receiving the results.
	     *
	     * @param fun
	     *			The function to apply to all objects
	     *			in the container.
	     *
	     * @param thread_count
	     *			The number of threads to spawn. If the
	     *			default value of -1 is specified, the
	     *			thread count is determined based on
	     *			the number of available hardware
	     *			threads. A value of 1 selects the
	     *			single-threaded algorithm.
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
	     *			might be four times the number of
	     *			threads. A value of 0 enforces
	     *			single-object processing.
	     */
	    TransformThreadPoolImpl(InputIterator& first, const Last& last,
				    OutputIterator& result, Function& fun,
				    int thread_count,
				    std::size_t maxpart)
		: queue(first, last, result, fun, maxpart),
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
	 * Run a function on all objects in a range of iterators, store
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
	 *		The function to apply to all objects in the
	 *		range.
	 *
	 * @returns
	 *		The final value of the result iterator
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
		 class InputIterator, class Last,
		 class OutputIterator, class Function,
		 class = typename std::enable_if<std::is_same<InputIterator,Last>::value ||
						 !std::is_integral<InputIterator>::value ||
						 !std::is_integral<Last>::value
						 >::type
		 >
	typename std::decay<OutputIterator>::type
	transform(InputIterator first, const Last& last,
		  OutputIterator result, Function&& fun)
	{
	    unsigned int tc =
		ThreadPoolImpl::impl::GenericThreadPool::determine_thread_count(thread_count);

	    if (tc <= 1) {
		return singlethreaded::parallel::transform(first, last, result, fun);
	    } else {
		ThreadPoolImpl::impl::TransformThreadPoolImpl<InputIterator, Last,
							      OutputIterator, Function
							      >(first, last, result, fun, thread_count,
								maxpart != 1 ? maxpart : 3 * (tc + 1));
		return result;
	    }
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
	 *
	 * @returns
	 *		The final value of the result iterator
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
	template<int thread_count =-1,
		 unsigned int maxpart = 1,
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
	 * Run a function on all objects in a container, store the return
	 * values through an output iterator.
	 *
	 * Version for lvalue containers
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
		 class Container, class OutputIterator, class Function>
	typename std::decay<OutputIterator>::type
	transform(Container& container,
		  OutputIterator&& result, Function&& fun)
	{
	    return transform<thread_count, maxpart
			     >(std::begin(container), std::end(container),
			       std::forward<OutputIterator>(result),
			       std::forward<Function>(fun));
	}

	/**
	 * Run a function on all objects in a container, store the return
	 * values through an output iterator.
	 *
	 * Version for rvalue containers
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

} // End of namespace threadpool

#endif // !defined(PARALLEL_TRANSFORM_H)
