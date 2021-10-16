/** @file threadpool/impl/threadpool_impl.h
 *
 * Threadpool for C++11, virtual thread pool implementation
 *
 * @copyright	2014 Ruediger Helsch, Ruediger.Helsch@t-online.de
 * @license	All rights reserved. Use however you want. No warranty at all.
 * $Revision: 2.0 $
 * $Date: 2014/05/14 16:56:58 $
 */
#ifndef THREADPOOL_IMPL_THREADPOOL_IMPL_H
#define THREADPOOL_IMPL_THREADPOOL_IMPL_H

#include <cstddef>
#include <memory>

#include "../threadpool_config.h"
#include "threadpool_interface_virtual.h"
#include "threadpool_generic.h"
#include "threadpool_impl_util.h"
#include "threadpool_impl_homogenous.h"

namespace ThreadPoolImpl {

    namespace impl {





	/**
	 * Implementation of virtual thread pool.
	 *
	 * Implements the functionality of the virtual thread
	 * pool. Only provides an interface to run a generic
	 * VirtualThreadPool_Task. The convenience functions to run
	 * different types of callable objects should be implemented
	 * in a subclass.
	 *
	 * The template parameter is not used, only serves to make
	 * this a class template which can be instantiated in multiple
	 * compilation units without giving multiply defined symbol
	 * errors.
	 */
	template<int = 0>
	class VirtualThreadPoolImpl : public VirtualThreadPoolInterface {

	    /**
	     * Store pointers into the queue. Decorate the pointers
	     * with an operator() to make them callable as needed by
	     * HomogenousThreadPool.
	     *
	     * I tried std::unique_ptr but at least with g++ it was
	     * very slow. Seems to do some heavyweight locking. Using
	     * raw pointers, we can just delete them in the tasks
	     * operator() or in the destructor. This is even more
	     * flexible than using std::unique_ptr, there may be use
	     * cases where the task shall outlive the execution by the
	     * thread pool, for example for cleanup work to be done.
	     *
	     * Delete copy-constructor and copy-assignment so `pimpl`
	     * is not deleted twice. The move constructor makes sure
	     * to leave an empty `pimpl` behind.
	     */
	    class QueueElement {
		VirtualThreadPool_Task* pimpl;
		QueueElement() = delete;
		QueueElement(const QueueElement&) = delete;
		QueueElement& operator=(const QueueElement&) = delete;
		QueueElement& operator=(QueueElement&&) = delete;
	    public:
		QueueElement(VirtualThreadPool_Task* t)	: pimpl(t) { }
		QueueElement(QueueElement&& x) : pimpl(x.pimpl) { x.pimpl = nullptr; }
		void operator()() { (*pimpl)(); pimpl = nullptr; }
		~QueueElement() { if (pimpl) delete pimpl; }
	    };

	    HomogenousThreadPool<QueueElement> impl;

	public:

	    typedef VirtualThreadPool_Task Task;

	    explicit VirtualThreadPoolImpl(int thread_count = -1,
					   std::size_t queue_size = 0,
					   std::size_t maxpart = 1)
		: impl(thread_count, queue_size, maxpart) { }

	    void run(std::unique_ptr<VirtualThreadPool_Task>&& t) {
		impl.run(t.release());
	    }

	    void run(VirtualThreadPool_Task* t) {
		impl.run(t);
	    }

	    void wait() { impl.wait(); }

	    void join() { impl.join(); }
	};

    } // End of namespace impl

} // End of namespace ThreadPoolImpl





#endif // !THREADPOOL_IMPL_THREADPOOL_IMPL_H
