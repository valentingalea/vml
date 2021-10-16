/** @file threadpool/threadpool.h
 *
 * Threadpool for C++11, header for thread pool
 *
 * @copyright	2014 Ruediger Helsch, Ruediger.Helsch@t-online.de
 * @license	All rights reserved. Use however you want. No warranty at all.
 * $Revision: 2.0 $
 * $Date: 2014/05/14 16:56:58 $
 */
#ifndef THREADPOOL_THREADPOOL_H
#define THREADPOOL_THREADPOOL_H

#include <cstddef>

#include "threadpool_config.h"
#include "impl/threadpool_interface_virtual.h"





namespace ThreadPoolImpl {

    namespace impl {

	/**
	 * Virtual thread pool 
	 *
	 * Builds a compiler firewall around VirtualThreadPoolImpl so
	 * that the thread pool can be used without seeing the
	 * internals.This also speeds up compilation because the
	 * compiler does not see the implementation.
	 *
	 * Defines only the implementation and not the usability
	 * member functions that make it easy to run tasks. The
	 * derived class ThreadPool defines these.
	 *
	 * This will only ever by used with template parameter 0. We
	 * could define the class directly, but then it would not be
	 * allowed to include the class definition in multiple
	 * separately compiled files. By making it a class *template*,
	 * we profit from the fact that multiple implicit
	 * instantiations of a template are allowed. This means when
	 * the user switches between header-only and library
	 * configuration he does not need to recompile everything, and
	 * the ODR is not violated.
	 */
	template<int = 0>
	class VirtualThreadPool : public VirtualThreadPoolInterface {

	    std::unique_ptr<VirtualThreadPoolInterface> pimpl;

	public:

	    typedef VirtualThreadPool_Task Task;



	    explicit VirtualThreadPool(int thread_count = -1,
				       std::size_t queue_size = 0,
				       std::size_t maxpart = 1);



	    void run(std::unique_ptr<Task>&& t) override;
	    void run(Task* t) override;



	    /**
	     * Wrap void functions in a task and run them without
	     * exception handling.
	     */
	    template<class Function>
	    typename std::enable_if<!std::is_pointer<typename std::remove_reference<Function>::type>::value &&
				    std::is_void<decltype(std::declval<typename std::remove_pointer<typename std::remove_reference<Function>::type>::type>()())>::value
				    >::type run(Function&& f) {
	    
		typedef typename std::remove_reference<Function>::type function_type;

		class WrappedFunction : public Task {
		    Function f;
		public:
		    WrappedFunction(function_type&& f) : f(std::move(f)) { }
		    virtual void operator()() override {
			f();
			delete this;
		    }
		};

		run(new WrappedFunction(std::forward<Function>(f)));
	    }



	    /**
	     * For functions with nonvoid return type, catch exceptions
	     * and return a future.
	     */
	    template<class Function>
	    typename std::enable_if<!std::is_pointer<typename std::remove_reference<Function>::type>::value &&
				    !std::is_void<decltype(std::declval<typename std::remove_pointer<typename std::remove_reference<Function>::type>::type>()())>::value,
				    std::future<decltype(std::declval<typename std::remove_pointer<typename std::remove_reference<Function>::type>::type>()())>
				    >::type run(Function&& f)  {

		typedef typename std::remove_reference<Function>::type function_type;
		typedef typename std::result_of<Function()>::type return_type;

		class WrappedFunction : public Task {

		    Function f;
		    std::promise<return_type> promise;

		public:

		    WrappedFunction(function_type&& f) : f(std::move(f)) { }
		    WrappedFunction(function_type& f) : f(f) { }

		    std::future<return_type> get_future() {
			return promise.get_future();
		    }

		    virtual void operator()() override {
			try {
			    promise.set_value(f());
			} catch (...) {
			    promise.set_exception(std::current_exception());
			}
			delete this;
		    }

		};

		WrappedFunction* task(new WrappedFunction(std::forward<Function>(f)));
		std::future<return_type> future(task->get_future());
		run(task);
		return future;
	    }



	    /**
	     * Run a function on all objects in an iterator range
	     *
	     * @param first
	     *			Start of the range
	     *
	     * @param last
	     *			End of the range
	     *
	     * @param fun
	     *			The function taking one parameter
	     *			by reference and returning void.
	     *
	     * Does not wait for all tasks to finish! Caller is
	     * responsible for wait()ing on the pool if necessary.
	     */
	    template<class Iterator, class Function>
	    void for_each(Iterator first, const Iterator& last, Function&& fun) {
		while (first != last) {
		    Wrap<typename iterval_traits<Iterator>::type> e(iterval_traits<Iterator>::copy(first));
		    ++first;
		    run([&fun,e](){
			    fun(iterval_traits<Iterator>::pass(std::move(e.value)));
			});;
		}
	    }



	    /**
	     * Run a function on all members of a container
	     *
	     * @param container
	     *			The container to process
	     *
	     * @param fun
	     *			The function taking one parameter
	     *			by reference and returning void.
	     *
	     * Does not wait for all tasks to finish! Caller is
	     * responsible for wait()ing on the pool if necessary.
	     */
	    template<class Container, class Function>
	    void for_each(Container&& container, Function&& fun) {
		for (auto& e: container)
		    run([&fun,&e](){
			    fun(e);
		    });
	    }



	    /**
	     * Wait for all active tasks to finish.
	     *
	     * Also throws an exception if one of the tasks has
	     * encountered an uncatched exception.
	     *
	     * Leaves the pool in a valid state ready to run more
	     * tasks, unless an exception has been thrown.
	     */
	    void wait() override;



	    /**
	     * Discard all tasks from the queue that have not yet
	     * started and wait for all threads to return.
	     *
	     * Also throws an exception if one of the tasks has
	     * encountered an uncatched exception.
	     *
	     * Leaves the pool in a shutdown state not ready to run
	     * tasks, but ready for destruction.
	     */
	    void join() override;



	    /**
	     * Destroy the thread pool.
	     *
	     * Does the equivalent of wait() and join() before the
	     * thread pool is destructed. This means, the destructor
	     * can hang a long time and can throw an exception (unless
	     * wait() or join() have been called before the
	     * destructor).
	     */
	    virtual ~VirtualThreadPool();

	};





    } // End of namespace impl

} // End of namespace ThreadPoolImpl






/*
  For library configuration hide internals behind compiler firewall.
  For header-only configuration reveal internals.
 */
#if defined(THREADPOOL_USE_LIBRARY) && THREADPOOL_USE_LIBRARY

     namespace ThreadPoolImpl {
	 namespace impl {
	     extern template class VirtualThreadPool<0>;
	     //extern template class ThreadPoolTmpl<0>;
	 }
     }

#else

#   include "impl/threadpool_inst.h" 

#endif





/*
  Export exported symbols to namespace `threadpool`.
*/
namespace threadpool {

    /**
     * Exported thread pool.
     */
    typedef ThreadPoolImpl::impl::VirtualThreadPool<> ThreadPool;

    /**
     * Thread pool with template parameters predefining the
     * constructor parameters.
     */
    template<int default_thread_count = -1,
	     std::size_t default_queue_size = 0,
	     std::size_t default_maxpart = 1>
    class ThreadPoolTemplate : public ThreadPool {
    public:

	ThreadPoolTemplate(int thread_count = default_thread_count,
			   std::size_t queue_size = default_queue_size,
			   std::size_t maxpart = default_maxpart)
	    : ThreadPool(thread_count, queue_size, maxpart) { }
    };

} // End of namespace threadpool





#endif // !defined(THREADPOOL_THREADPOOL_H)
