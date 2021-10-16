/** @file threadpool/impl/threadpool_generic.h
 *
 * Threadpool for C++11, header for generic pool
 *
 * @copyright	2014 Ruediger Helsch, Ruediger.Helsch@t-online.de
 * @license	All rights reserved. Use however you want. No warranty at all.
 * $Revision: 2.0 $
 * $Date: 2014/05/14 16:56:58 $
 */
#ifndef THREADPOOL_IMPL_THREADPOOL_GENERIC_H
#define THREADPOOL_IMPL_THREADPOOL_GENERIC_H

#include <memory>

#include "../threadpool_config.h"
#include "threadpool_interface_generic.h"





namespace ThreadPoolImpl {

    namespace impl {





	/**
	 * A thread pool reading its tasks from a GenericThreadPoolQueue.
	 *
	 * The template parameter is ignored. This will only ever by
	 * used with template parameter 0. We could define a straight
	 * non-template class, but then it would not be possible to
	 * include the class definition in multiple separately
	 * compiled files. By making it a class *template*, we profit
	 * from the fact that multiple implicit instantiations of a
	 * template are allowed. This means when the user switches
	 * between header-only and library configuration he does not
	 * need to recompile everything, and the ODR is not violated.
	 */
	template<int = 0>
	class GenericThreadPoolTmpl
	    : public GenericThreadPoolInterface<GenericThreadPoolQueue> {

	    std::unique_ptr<GenericThreadPoolInterface<GenericThreadPoolQueue>> pimpl;

	public:

	    /**
	     * Create a generic thread pool.
	     *
	     * As soon as the pool is created the threads start
	     * running tasks from the queue until the queue is
	     * empty. When the queue is empty the threads return and
	     * are ready to be collected by join() or by the
	     * destructor.
	     *
	     * @param queue
	     *			The queue managing the tasks.
	     *
	     * @param thread_count
	     *			The number of threads to use. If the
	     *			thread count is not specified it
	     *			defaults to the number of available
	     *			hardware threads
	     *			std::thread::hardware_concurrency(),
	     *			as read through
	     *			hardware_concurrency().
	     *
	     */
	    GenericThreadPoolTmpl(GenericThreadPoolQueue& queue,
				  int thread_count = -1);

	    /**
	     * Help with the work.
	     *
	     * @param return_if_idle
	     *		Never wait for work, return instead.
	     */
	    void help(bool return_if_idle) override;

	    /**
	     * Rethrow a potentially pending exception from a worker
	     * thread.
	     */
	    void rethrow_exception() override;

	    /**
	     * Wait for all threads to finish and collect them.
	     *
	     * Leaves the thread pool ready for destruction.
	     */
	    void join() override;

	    /**
	     * Destroy the thread pool.
	     *
	     * Generally a destructor should not wait for a long time,
	     * and it should not throw any exceptions. Unfortunately
	     * threads are not abortable in C++.  The only way to make
	     * sure the threads have terminated is to wait for them to
	     * return by themselves.  If we would allow the
	     * destruction of the thread pool before the threads have
	     * returned, the threads would continue to access the
	     * memory of the destroyed thread pool, potentially
	     * clobbering other objects residing in the recycled
	     * memory.  We could allocate parts of the memory with
	     * new, and leave it behind for the threads after the
	     * thread pool is destructed.  But even then, the user
	     * supplied functions run by the threads might access
	     * memory that gets destroyed if the function that
	     * constructed the thread pool terminates.  The danger of
	     * undetected and undebuggable memory corruption is just
	     * too big.
	     *
	     * With regard to the exceptions rethrown in the destructor,
	     * it is better to signal the exception than to ignore it
	     * silently.
	     *
	     * If it is not acceptable for the destructor to wait or
	     * to throw an exception, just call join() before the pool
	     * is destructed.  After join() returns the destructor is
	     * guaranteed to run fast and without exceptions.
	     *
	     * If it should really be necessary to keep threads
	     * running after the function that created the thread pool
	     * returns, just create the thread pool on the heap with
	     * new. And if you want to make sure nobody destroys the
	     * thread pool, feel free to throw away the handle.
	     */
	    virtual ~GenericThreadPoolTmpl();

	    /**
	     * Cache the hardware concurrency so we are sure that it
	     * is cheap to get. Also this gives us a point to
	     * cheat. The cached value can be modified by a parameter.
	     *
	     * @param c
	     *		The hardware concurrency to use
	     */
	    static unsigned int hardware_concurrency(int c = -1);

	    /**
	     * Determine thread count to use based on users
	     * specifications.
	     *
	     * @param thread_count
	     *		Runtime specified threadcount parameter.
	     *
	     * @returns
	     *		The number of threads to use. Returns always a
	     *		positive number, possibly 1 for
	     *		single-processor systems.
	     *
	     * This policy function does just some
	     * guesswork. Allocating a number of threads in the order
	     * of the hardware threads may be a good bet for CPU-bound
	     * work. For other tasks it depends.
	     */
	    static unsigned int determine_thread_count(int thread_count = -1);

	    /**
	     * Switch exception handling off
	     */
	    static bool ignore_thread_pool_exceptions(bool i = true);

	};





	typedef GenericThreadPoolTmpl<> GenericThreadPool;





    } // End of namespace impl

} // End of namespace ThreadPoolImpl





/*
  For library configuration hide internals behind compiler firewall.
  For header-only configuration reveal internals.
 */
#if defined(THREADPOOL_USE_LIBRARY) && THREADPOOL_USE_LIBRARY

     namespace ThreadPoolImpl {
	 namespace impl {
	     extern template class GenericThreadPoolTmpl<>;
	 }
     }

#else // !THREADPOOL_USE_LIBRARY

#   include "threadpool_generic_inst.h"

#endif // !THREADPOOL_USE_LIBRARY





#endif // !defined(THREADPOOL_IMPL_THREADPOOL_GENERIC_H)
