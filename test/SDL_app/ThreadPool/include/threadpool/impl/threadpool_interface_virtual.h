/** @file threadpool/impl/threadpool_interface_virtual.h
 *
 * Threadpool for C++11, interface of virtual pool
 *
 * @copyright	2014 Ruediger Helsch, Ruediger.Helsch@t-online.de
 * @license	All rights reserved. Use however you want. No warranty at all.
 * $Revision: 2.0 $
 * $Date: 2014/05/14 16:56:58 $
 */
#ifndef THREADPOOL_IMPL_THREADPOOL_INTERFACE_VIRTUAL_H
#define THREADPOOL_IMPL_THREADPOOL_INTERFACE_VIRTUAL_H

#include <cstddef>
#include <memory>
#include <utility>
#include <type_traits>
#include <future>

#include "../threadpool_config.h"
#include "threadpool_impl_util.h"

namespace ThreadPoolImpl {

    namespace impl {





	/**
	 * Task type for virtual thread pool
	 *
	 * The virtual thread pool wraps functions into object of type
	 * VirtualThreadPool_Task and enqueues them.  the actual tasks
	 * can be heterogenous and must only support the
	 * VirtualThreadPool_Task interface.
	 */
	class VirtualThreadPool_Task {
	public:

	    /**
	     * The payload, users function to be run.
	     *
	     * Operator() is run from the thread pool. Is responsible
	     * for deleting the task object once it is done.
	     */
	    virtual void operator()() = 0;

	    /**
	     * Destroy the task object
	     */
	    virtual ~VirtualThreadPool_Task() {};
	};





	/**
	 * Interface of virtual thread pool.
	 *
	 * Implements the functionality of the virtual thread pool. Only
	 * provides an interface to run a generic
	 * VirtualThreadPool_Task. The convenience functions to run
	 * different types of callable objects must be implemented in a
	 * subclass.
	 *
	 */
	class VirtualThreadPoolInterface {

	public:

	    typedef VirtualThreadPool_Task Task;

	    virtual void run(std::unique_ptr<VirtualThreadPool_Task>&& c) = 0;
	    virtual void run(VirtualThreadPool_Task* c) = 0;

	    /**
	     * Wait for all active tasks to finish.
	     *
	     * Also throws an exception if one of the tasks has encountered
	     * an uncatched exception.
	     *
	     * Leaves the pool in a valid state ready to run more tasks, unless
	     * an exception has been thrown.
	     */
	    virtual void wait() = 0;

	    /**
	     * Discard all tasks from the queue that have not yet started
	     * and wait for all threads to return.
	     *
	     * Also throws an exception if one of the tasks has encountered
	     * an uncatched exception.
	     *
	     * Leaves the pool in a shutdown state not ready to run tasks,
	     * but ready for destruction.
	     */
	    virtual void join() = 0;

	    /**
	     * Destroy the thread pool.
	     *
	     * Does the equivalent of wait() and join() before the thread pool
	     * is destructed. This means, the destructor can hang a long time
	     * and can throw an exception (unless wait() or join() have been
	     * called before the destructor).
	     */
	    virtual ~VirtualThreadPoolInterface() { };
	};





	/**
	 * User interface for virtual thread pool.
	 *
	 * Adds convenience functions to the virtual thread pool base so
	 * that different types of callable objects can be executed.
	 *
	 * @tparam Base
	 *		The virtual thread pool 
	 */
	template<class Base>
	class VirtualThreadPoolUserInterface : public Base {

	public:



#if !defined(__GNUC__) || __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 7)
	    using Base::Base;
#else // GCC 4.7 does not support inheriting constructors
	    explicit VirtualThreadPoolUserInterface(unsigned t=0,std::size_t q=0,std::size_t p=1):Base(t,q,p){}
#endif



	    using Base::run;



	    /**
	     * Wrap void functions in a task and run them without
	     * exception handling.
	     */
	    template<class Function>
	    typename std::enable_if<is_callable<Function>::value
				    && std::is_void<typename std::result_of<Function()>::type>::value,
				    void
				    >::type run(Function&& f) {
	    
		typedef typename std::remove_reference<Function>::type function_type;

		class WrappedFunction : public Base::Task {
		    Function f;
		public:
		    WrappedFunction(function_type&& f) : f(std::move(f)) { }
		    virtual void operator()() override {
			f();
			delete this;
		    }
		};

		Base::run(new WrappedFunction(std::forward<Function>(f)));
	    }



	    /**
	     * For functions with nonvoid return type, catch exceptions
	     * and return a future.
	     */
	    template<class Function>
	    typename std::enable_if<is_callable<Function>::value
				    && !std::is_void<typename std::result_of<Function()>::type>::value,
				    std::future<typename std::result_of<Function()>::type>
				    >::type run(Function&& f) {

		typedef typename std::remove_reference<Function>::type function_type;
		typedef typename std::result_of<Function()>::type return_type;

		class WrappedFunction : public Base::Task {

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
		Base::run(task);
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

	};





    } // End of namespace impl

} // End of namespace ThreadPoolImpl





#endif // !defined(THREADPOOL_IMPL_THREADPOOL_INTERFACE_VIRTUAL_H)
