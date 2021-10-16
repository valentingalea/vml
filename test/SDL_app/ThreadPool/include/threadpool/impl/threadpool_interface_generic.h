/** @file threadpool/impl/threadpool_interface_generic.h
 *
 * Threadpool for C++11, interface of generic pool
 *
 * @copyright	2014 Ruediger Helsch, Ruediger.Helsch@t-online.de
 * @license	All rights reserved. Use however you want. No warranty at all.
 * $Revision: 2.0 $
 * $Date: 2014/05/14 16:56:58 $
 */
#ifndef THREADPOOL_INTERFACE_GENERIC_H
#define THREADPOOL_INTERFACE_GENERIC_H

#include <memory>

#include "../threadpool_config.h"

namespace ThreadPoolImpl {

    namespace impl {





	/**
	 * A Queue interface for the generic thread pool.
	 */
	class GenericThreadPoolQueue {
	public:

	    /**
	     * Work on items in the queue. If there are no more items to
	     * work on, but more items could possible be added, then wait
	     * for new items unless parameter return_if_idle is true.
	     *
	     * @param return_if_idle
	     *		Return if there is no more work in the moment.
	     */
	    virtual void work(bool return_if_idle) = 0;

	    /**
	     * Shut the queue down.
	     *
	     * Irreversably terminates processing. All idle workers should be
	     * woken up and return from work() as soon as the current task
	     * is processed.
	     */
	    virtual void shutdown() = 0;
	    virtual ~GenericThreadPoolQueue() { };
	};





	/**
	 * A generic thread pool interface.
	 *
	 * @tparam class Queue
	 *			The queue delivering the tasks.
	 *
	 *	Class Queue must provide the following members:
	 *
	 *	- void work()
	 *			Gets tasks and works until the end of the
	 *			queue is reached.
	 *	- void shutdown()
	 *			Causes the queue to return invalid tasks,
	 *			which will cause all threads to return.
	 *
	 *
	 * As soon as the pool is created the threads start running tasks
	 * from the queue until the queue is empty. When the queue is
	 * empty the threads return and are ready to be collected by
	 * join() or by the destructor. The following constructor arguments
	 * should be supported:
	 *
	 * - Queue& queue
	 *			The queue delivering the tasks.
	 *
	 * - int thread_count
	 *			The number of threads to use. If the
	 *			thread count is not specified (default
	 *			value -1) it defaults to the number of
	 *			available hardware threads
	 *			std::thread::hardware_concurrency(),
	 *			as read through
	 *			hardware_concurrency().
	 */
	template<class Queue>
	class GenericThreadPoolInterface {

	public:

	    GenericThreadPoolInterface() { }

	    /**
	     * Help with the work.
	     *
	     * @param return_if_idle
	     *		Never wait for work, return instead.
	     *
	     * This function is called by all threads wanting to help.
	     */
	    virtual void help(bool return_if_idle) = 0;

	    /**
	     * Rethrow a potentially pending exception from a worker
	     * thread.
	     */
	    virtual void rethrow_exception() = 0;

	    /**
	     * Wait for all threads to finish and collect them.
	     *
	     * Leaves the thread pool ready for destruction.
	     */
	    virtual void join() = 0;

	    /**
	     * Destroy the thread pool.
	     *
	     * Generally a destructor should not wait for a long time, and
	     * it should not throw any exceptions. Unfortunately threads
	     * are not abortable in C++.  The only way to make sure the
	     * threads have terminated is to wait for them to return by
	     * themselves.  If we would allow the destruction of the thread
	     * pool before the threads have returned, the threads would
	     * continue to access the memory of the destroyed thread pool,
	     * potentially clobbering other objects residing in the
	     * recycled memory.  We could allocate parts of the memory
	     * with new, and leave it behind for the threads after the
	     * thread pool is destructed.  But even then, the user supplied
	     * functions run by the threads might access memory that gets
	     * destroyed if the function that constructed the thread pool
	     * terminates.  The danger of undetected and undebuggable
	     * memory corruption is just too big.
	     *
	     * With regard to the exceptions rethrown in the destructor,
	     * it is better to signal the exception than to ignore it
	     * silently.
	     *
	     * If it is not acceptable for the destructor to wait or to
	     * throw an exception, just call join() before the pool is
	     * destructed.  After join() the destructor is guaranteed to
	     * run fast and without exceptions.
	     *
	     * If it should really be necessary to keep threads running
	     * after the function that created the thread pool returns,
	     * just create the thread pool on the heap with new. And if
	     * you want to make sure nobody destroys the thread pool, feel
	     * free to throw away the handle.
	     *
	     * It is not possible to use the default nothrow version.
	     */
	    virtual ~GenericThreadPoolInterface() { }
	};





    } // End of namespace impl

} // End of namespace ThreadPoolImpl






#endif // !defined(THREADPOOL_INTERFACE_GENERIC_H)
