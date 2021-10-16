/** @file threadpool/singlethreaded/threadpool.h
 *
 * Threadpool for C++11, singlethreaded pool
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
 * $Date: 2014/05/14 16:56:59 $
 */
#ifndef THREADPOOL_SINGLETHREADED_THREADPOOL_H
#define THREADPOOL_SINGLETHREADED_THREADPOOL_H

#include <cstddef>
#include <type_traits>
#include <memory>
#include <utility>
#include <exception>
#include <future>

#include "../impl/threadpool_impl_util.h"
#include "../impl/threadpool_interface_virtual.h"

namespace threadpool {

    namespace singlethreaded {





	class ThreadPool {

	public:

	    typedef ThreadPoolImpl::impl::VirtualThreadPool_Task Task;

	    explicit ThreadPool(int = 1, std::size_t = 1, unsigned int = 1) { }

	    void run(Task* task) { task->operator()(); delete task; }
	    void run(std::unique_ptr<Task> task) { Task* t(task.release()); t->operator()(); delete t; }

	    template<class Function>
	    typename std::enable_if<ThreadPoolImpl::impl::is_callable<Function>::value
				    && std::is_void<typename std::result_of<Function()>::type>::value,
				    void
				    >::type run(Function&& fun)
	    {
		fun();
	    }

	    template<class Function>
	    typename std::enable_if<ThreadPoolImpl::impl::is_callable<Function>::value
				    && !std::is_void<typename std::result_of<Function()>::type>::value,
				    std::future<typename std::result_of<Function()>::type>
				    >::type run(Function&& fun)
	    {
		typedef typename std::result_of<Function()>::type return_type;
		std::promise<return_type> promise;
		try {
		    promise.set_value(fun());
		} catch (...) {
		    promise.set_exception(std::current_exception());
		}
		return promise.get_future();
	    }

	    template<typename Container, typename Function>
	    void for_each(Container&& container, Function&& fun) {
		for (auto& e: container)
		    fun(e);
	    }

	    void wait() {}
	    void join() {}
	    void shutdown() {}
	};





	/**
	 * Thread pool with template parameters predefining the
	 * constructor parameters, singlethreaded version
	 *
	 * The parameters are ignored and are only there for call
	 * compatibility with the multithreaded version.
	 */
	template<int default_thread_count = 1,
		 std::size_t default_queue_size = 1,
		 unsigned int default_maxpart = 1>
	class ThreadPoolTemplate : public ThreadPool {
	public:

#if !defined(__GNUC__) || __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 7)
	    using ThreadPool::ThreadPool;
#else // GCC 4.7 does not support inheriting constructors
	    explicit ThreadPoolTemplate(int t=default_thread_count,std::size_t q=default_queue_size,unsigned p=default_maxpart):ThreadPool(t,q,p){}
#endif
	};





    } // End of namespace singlethreaded

} // End of namespace threadpool

#endif // !defined(THREADPOOL_SINGLETHREADED_THREADPOOL_H)
