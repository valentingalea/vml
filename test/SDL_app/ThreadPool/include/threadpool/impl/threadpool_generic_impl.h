/** @file threadpool/impl/threadpool_generic_impl.h
 *
 * Threadpool for C++11, generic pool implementation
 *
 * @copyright	2014 Ruediger Helsch, Ruediger.Helsch@t-online.de
 * @license	All rights reserved. Use however you want. No warranty at all.
 * $Revision: 2.0 $
 * $Date: 2014/05/14 16:56:58 $
 */
#ifndef THREADPOOL_IMPL_THREADPOOL_GENERIC_IMPL_H
#define THREADPOOL_IMPL_THREADPOOL_GENERIC_IMPL_H

#include <limits>
#include <type_traits>
#include <functional>		// For std::bind()
#include <vector>
#include <thread>
#include <mutex>

#include "../threadpool_config.h"
#include "threadpool_interface_generic.h"





#ifdef _MSC_VER // Work around Visual C++ bug, defines macros min() and max()
#   ifdef min
#	undef min
#   endif
#   ifdef max
#	undef max
#   endif
#endif





namespace ThreadPoolImpl {

    namespace impl {





	/**
	 * A thread pool reading its tasks from a generic queue.
	 *
	 * @tparam class Queue
	 *			The queue delivering the tasks.
	 *
	 *	Class Queue must provide the following members:
	 *
	 *	- void work()
	 *			Gets tasks and works until the end of
	 *			the queue is reached.
	 *
	 *	- void shutdown()
	 *			Causes the queue to tell the threads
	 *			asking for work to return.
	 *
	 */
	template<class Queue>
	class GenericThreadPoolImpl : public GenericThreadPoolInterface<Queue> {



	    class Worker {
		std::thread thread;
	    public:
		Worker& operator=(std::thread&& t) { thread = std::move(t); return *this; }
		void join() { if (thread.joinable()) thread.join(); }
	    };



	    std::mutex mutex;
	    std::exception_ptr pending_exception;
	    Queue& queue;

	    const unsigned int thread_count; /// The number of threads

	    std::vector<Worker> workers;



	    /**
	     * The main function of the thread.
	     */
	    void work() {
		help(false);
	    }



	    /**
	     * Wait for all workers to finish.
	     */
	    void join_workers() {
		work();		// Instead of hanging around, help the workers!
		for (Worker& w: workers)
		    w.join();
	    }


	    // Copying and moving are not supported.
	    GenericThreadPoolImpl(const GenericThreadPoolImpl&) = delete;
	    GenericThreadPoolImpl(GenericThreadPoolImpl&&) = delete;
	    GenericThreadPoolImpl& operator=(const GenericThreadPoolImpl&) = delete;
	    GenericThreadPoolImpl& operator=(GenericThreadPoolImpl&&) = delete;



	public:

	    /**
	     * Generic thread pool.
	     *
	     * As soon as the pool is created the threads start running
	     * tasks from the queue until the queue is empty. When the
	     * queue is empty the threads return and are ready to be
	     * collected by join() or by the destructor.
	     *
	     * @param queue
	     *			The queue delivering the tasks.
	     *
	     * @param thread_count The number of threads
	     *			to use. If the thread count is not
	     *			specified it defaults to the number of
	     *			available hardware threads
	     *			std::thread::hardware_concurrency(),
	     *			as read through hardware_concurrency().
	     *
	     */
	    GenericThreadPoolImpl(Queue& queue, int thread_count)
		: pending_exception(nullptr),
		queue(queue),
		thread_count(determine_thread_count(thread_count)),
		workers(this->thread_count) {
		for (Worker& w: workers)
		    w = std::move(std::thread(std::bind(&GenericThreadPoolImpl::work, this)));
	    }



	    /**
	     * Help with the work.
	     *
	     * @param return_if_idle
	     *		Never wait for work, return instead.
	     */
	    void help(bool return_if_idle) override {
		if (ignore_thread_pool_exceptions()) {
		    queue.work(return_if_idle);
		} else {
		    try {
			queue.work(return_if_idle);
		    } catch (...) {
			{
			    std::exception_ptr e = std::current_exception();
			    std::lock_guard<std::mutex> lock(mutex);
			    if (!pending_exception)
				pending_exception = std::move(e);
			}
			queue.shutdown();
		    }
		}
	    }



	    /**
	     * Rethrow a potentially pending exception from a worker
	     * thread.
	     */
	    void rethrow_exception() override {
		if (pending_exception && !std::uncaught_exception()) {
		    queue.shutdown();
		    join_workers();
		    if (!std::uncaught_exception()) {
			std::exception_ptr e = pending_exception;
			pending_exception = nullptr;
			std::rethrow_exception(std::move(e));
		    }
		}
	    }



	    /**
	     * Wait for all threads to finish and collect them.
	     *
	     * Leaves the thread pool ready for destruction.
	     */
	    void join() override {
		join_workers();
		rethrow_exception();
	    }



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
	     * With regard to the exceptions rethrown in the
	     * destructor, it is better to signal the exception than
	     * to ignore it silently.
	     *
	     * If it is not acceptable for the destructor to wait or
	     * to throw an exception, just call join() before the pool
	     * is destructed.  After join() the destructor is
	     * guaranteed to run fast and without exceptions.
	     *
	     * If it should really be necessary to keep threads
	     * running after the function that created the thread pool
	     * returns, just create the thread pool on the heap with
	     * new. And if you want to make sure nobody destroys the
	     * thread pool, feel free to throw away the handle.
	     */
	    virtual ~GenericThreadPoolImpl() {

		// Abort processing if destructor runs during exception handling.
		if (std::uncaught_exception())
		    queue.shutdown();

		join(); // Running threads would continue to access the destructed pool.
	    }



	    /**
	     * Cache the hardware concurrency so we are sure that it
	     * is cheap to get. Also this gives us a point to
	     * cheat. The cached value can be modified by a parameter.
	     *
	     * @param c
	     *		The hardware concurrency to use
	     */
	    static unsigned int hardware_concurrency(int c = -1) {
		static int cached_concurrency = -1;
		if (c != -1)
		    cached_concurrency = c;
		if (cached_concurrency == -1)
		    cached_concurrency = std::thread::hardware_concurrency();
		return cached_concurrency;
	    }



	    /**
	     * Determine thread count to use based on users
	     * specifications.
	     *
	     * @param thread_count
	     *		Runtime specified threadcount parameter.
	     *
	     * @returns
	     *		The number of threads to use.
	     *
	     * This policy function does just some
	     * guesswork. Allocating a number of threads in the order
	     * of the hardware threads may be a good bet for CPU-bound
	     * work. For other tasks it depends.
	     */
	    static unsigned int determine_thread_count(int thread_count = -1) {
		if (thread_count == -1
		    && !(thread_count = hardware_concurrency()))
		    thread_count = 8;
		return thread_count;
	    }



	    /**
	     * Switch exception handling off
	     */
	    static bool ignore_thread_pool_exceptions(bool i = true) {
		static bool do_ignore_exceptions = false;
		if (i)
		    do_ignore_exceptions = i;
		return do_ignore_exceptions;
	    }
	};





    } // End of namespace impl

} // End of namespace ThreadPoolImpl

#endif // !defined(THREADPOOL_IMPL_THREADPOOL_GENERIC_IMPL_H)
