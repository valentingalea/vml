/** @file threadpool/impl/threadpool_impl_homogenous.h
 *
 * Threadpool for C++11, homogenous thread pool
 *
 * @copyright	2014 Ruediger Helsch, Ruediger.Helsch@t-online.de
 * @license	All rights reserved. Use however you want. No warranty at all.
 * $Revision: 2.0 $
 * $Date: 2014/05/14 16:56:58 $
 */
#ifndef THREADPOOL_IMPL_THREADPOOL_IMPL_HOMOGENOUS_H
#define THREADPOOL_IMPL_THREADPOOL_IMPL_HOMOGENOUS_H

#include <cassert>
#include <limits>
#include <cstddef>
#include <type_traits>
#include <vector>
#include <mutex>
#include <condition_variable>

#include "threadpool_generic.h"
#include "threadpool_impl_util.h"






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
	 * Thread pool with homogenous functions
	 *
	 * This thread pool is dependent on template parameter
	 * Function. Only one type of functions can be queued. For
	 * example if this thread pool is instantiated for a lambda
	 * function, only this exact lambda function can be run,
	 * since each lambda function has its own type separate from
	 * all other lambda functions.
	 *
	 * To make this thread pool more universally usable,
	 * instantiate it either with std::function<void()>, or use
	 * the virtual thread pool VirtualThreadPool.
	 *
	 * @tparam Function
	 *		The function type to be used for the
	 *		queue. All tasks to be queued must be of this
	 *		type. The class Function must support the
	 *		following interface:
	 *		- operator()(): the functions must be callable
	 *		  with no parameters.
	 */
	template<class Function>
	class HomogenousThreadPool {

	    /**
	     * Queue for functions with signature void()
	     *
	     * This queue is dependent on template parameter
	     * Function. Only one type of functions can be queued. For
	     * example if this queue is instantiated for a lambda
	     * function, only this exact lambda function can be
	     * queued, since each lambda function has its own type
	     * separate from all other lambda functions.
	     *
	     * To make this queue more flexible, instantiate it either
	     * with std::function<void()>, or use the virtual thread
	     * pool VirtualThreadPool.
	     *
	     * @tparam Function
	     *		The function type to queue.
	     */
	    class HQueue : public GenericThreadPoolQueue {

		/*
		  If we would use a deque, we would have to protect
		  against overlapping accesses to the front and the
		  back. The standard containers do not allow this. Use a
		  vector instead.  With a vector it is possible to access
		  both ends of the queue at the same time, as push()ing
		  and pop()ing does not modify the container itself but
		  only its elements.
		*/
		class Queue {
		    union Fun {
#ifdef _MSC_VER // Work around Visual C++ bug, does not like constructable objects in unions
			alignas(Function) char fun[sizeof(Function)];
#else // Standard conforming, C++11 9.5
			Function fun; // Only used between pop_ptr and push_ptr
#endif
			Fun() noexcept { }
			Fun(const Fun&) noexcept { }
			Fun(Fun&&) noexcept { }
			~Fun() noexcept { }
		    };
		    std::vector<Fun> impl;
		    std::size_t push_ptr = 0, pop_ptr = 0;

		    Queue(const Queue&) = delete;
		    Queue(Queue&&) = delete;
		    Queue& operator=(const Queue&) = delete;
		    Queue& operator=(Queue&&) = delete;

		public:

		    Queue(std::size_t s) : impl(s + 1) { }

		    template<class F> void push(F&& f) {
			new (&impl[push_ptr].fun) Function(std::forward<F>(f));
			if (++push_ptr == impl.size()) push_ptr = 0;
		    }

		    Function pop() {
#ifdef _MSC_VER // Work around Visual C++ bug, does not like constructable objects in unions
			Function r = std::move(reinterpret_cast<Function&>(impl[pop_ptr].fun));
			reinterpret_cast<Function&>(impl[pop_ptr].fun).~Function();
#else
			Function r = std::move(impl[pop_ptr].fun);
			impl[pop_ptr].fun.~Function();
#endif
			if (++pop_ptr == impl.size()) pop_ptr = 0;
			return r;
		    }

		    std::size_t size() const {
			std::size_t r = push_ptr + impl.size() - pop_ptr;
			if (r >= impl.size()) r -= impl.size();
			return r;
		    }

		    bool empty() const {
			return push_ptr == pop_ptr;
		    }

		    std::size_t capacity() {
			return impl.size() - 1;
		    }

		    void reserve(std::size_t s) {
			assert(empty()); // Copying / moving of Fun not supported.
			if (s >= impl.size())
			    impl.resize(s + 1);
		    }

		    ~Queue() { while (!empty()) pop(); }
		};



		/*
		  This queue requires attention for protection against
		  concurrent access. Protect against:
		  - Concurrent access by two worker threads both
		    wanting to get() a task from the queue at the same
		    time.
		  - Concurrent access by two threads both wanting to
		    put() a task into the queue at the same time.
		  - A worker thread having determined that the queue
		    is empty, while at the same time a new task is put()
		    into the queue.
		  - A task wanting to put() a task into the queue
		    having found the queue full, while at the same time
		    the queues fill level decreases.
		*/

		const std::size_t queue_size;
		const std::size_t maxpart;
		bool shutting_down;
		unsigned int idle_workers;
		unsigned int total_workers;
		bool wakeup_is_pending;
		Queue queue;
		std::mutex pop_mutex;
		std::mutex push_mutex;
		std::condition_variable waiting_workers;
		std::condition_variable waiters;
 
		/**
		 * Get tasks and execute them. Return as soon as the queue
		 * shrinks to `return_if_idle` tasks.
		 */
		void help(std::ptrdiff_t return_if_idle) {

		    std::size_t min_queue_size = return_if_idle < 0 ? 0 : return_if_idle;

		    // Increment total worker count, decrement again on scope exit
		    { std::lock_guard<std::mutex> lock(push_mutex); ++total_workers; }
		    //std::cerr << " total_workers(" << this->total_workers << ")";
		    auto x1 = at_scope_exit([this](){
			    std::lock_guard<std::mutex> lock(push_mutex);
			    if (--this->total_workers == this->idle_workers)
				this->waiters.notify_all();;
			});

		    Queue functions(1);

		    for (;;) {
			std::unique_lock<std::mutex> lock(pop_mutex);

			std::size_t queue_size;

			// Try to get the next task(s)
			while ((queue_size = queue.size()) <= min_queue_size) {
			    if (static_cast<std::ptrdiff_t>(queue_size) <= return_if_idle)
				return;
			    if (queue_size)
				break;

			    // The queue is empty, wait for more tasks to be put()

			    lock.unlock();

			    {
				std::unique_lock<std::mutex> lock(push_mutex);
				while (queue.empty() && !shutting_down) {

				    if (++idle_workers == total_workers)
					waiters.notify_all();;

				    waiting_workers.wait(lock); // Wait for task to be queued
				    wakeup_is_pending = false;

				    --idle_workers;
				}

			    }

			    if (shutting_down)
				return;

			    lock.lock();
			}

			// There is at least one task in the queue and the back is locked.

			std::size_t stride =
			    (maxpart == 0) ? 1 : queue_size / maxpart;
			if (stride <= 0)
			    stride = 1;
			if (stride > functions.capacity())
			    functions.reserve(2 * stride);

			while (stride--)
			    functions.push(queue.pop());

			lock.unlock();

			if (idle_workers && !wakeup_is_pending && queue_size)
			    waiting_workers.notify_one();

			while (!functions.empty())
			    functions.pop()();
		    }
		}

		/**
		 * Help, and shut down if an exception escapes.
		 */
		void try_help(std::ptrdiff_t return_if_idle) {
		    try {
			help(return_if_idle);
		    } catch (...) {
			shutdown();
			throw;
		    }
		}



	    public:

		HQueue(std::size_t queue_size, std::size_t maxpart)
		    : queue_size(queue_size ? queue_size : 10000),
		      maxpart(maxpart),
		      shutting_down(false),
		      idle_workers(0),
		      total_workers(0),
		      wakeup_is_pending(false),
		      queue(this->queue_size)
		{ }

		/**
		 * Get tasks and execute them. If `return_if_idle`, return
		 * instead of idly waiting.
		 */
		void work(bool return_if_idle) override {
		    help(return_if_idle ? 0 : -1);
		}

		/**
		   Enqueue a task.
		*/
		template<class C>
		void put(C&& c) {

		    std::unique_lock<std::mutex> lock(push_mutex);

		    while (queue.size() >= queue_size) {
			// No space in the queue. Must wait for workers to advance.

			lock.unlock();

			try_help(queue_size / 2);

			lock.lock();
		    }

		    // Now there is space in the queue and we have locked the back.

		    // Enqueue function.
		    if (shutting_down) {

			Function fun(std::forward<C>(c)); // Run Function destructor

		    } else {
			/*
			  Here we have exclusive access to the head of the
			  queue.
			*/
			queue.push(std::forward<C>(c));
		
			if (idle_workers && !wakeup_is_pending) {
			    wakeup_is_pending = true;
			    waiting_workers.notify_one();
			}
		    }
		}

		void shutdown() override {
		    std::unique_lock<std::mutex> push_lock(push_mutex);
		    std::unique_lock<std::mutex> pop_lock(pop_mutex);
		    shutting_down = true;
		    while (!queue.empty())
			queue.pop();
		    waiting_workers.notify_all();
		    waiters.notify_all();
		}

		void wait() {
		    if (std::uncaught_exception())
			shutdown();
		    std::exception_ptr e;
		    std::unique_lock<std::mutex> lock(push_mutex);
		    while (!queue.empty() || idle_workers != total_workers) {
			while (!queue.empty()) {
			    lock.unlock();
			    try {
				try_help(0);
			    } catch (...) {
				if (e == nullptr)
				    e = std::current_exception();
			    }
			    lock.lock();
			}
			while (idle_workers != total_workers)
			    waiters.wait(lock);
		    }
		    if (e != nullptr && !std::uncaught_exception())
			std::rethrow_exception(std::move(e));
		}
	    };

	    HQueue queue;
	    GenericThreadPool pool;

	public:

	    explicit HomogenousThreadPool(int thread_count = -1,
					  std::size_t queue_size = 0,
					  std::size_t maxpart = 1)
		: queue(queue_size,
			(maxpart != 1) ? maxpart
			: 3 * (GenericThreadPool::determine_thread_count(thread_count)+ 1)),
		  pool(queue, thread_count)
	    { }

	    template<class F>
	    void run(F&& f) {
		queue.put(std::forward<F>(f));
	    }

	    void wait() {
		pool.help(true); 	// Help out instead of sitting around idly.
		queue.wait();
	    }

	    void join() {
		queue.shutdown();
		pool.join();
	    }

	    virtual ~HomogenousThreadPool() {
		wait();
		join();
	    }

	    /**
	     * Run a function on all members of a container
	     *
	     * @param container
	     *			The container to process
	     * @param fun
	     *			The function taking one parameter
	     *			by reference and returning void.
	     *
	     * Does not wait for all tasks to finish! Caller is
	     * responsible for wait()ing on the pool if necessary.
	     */
	    template<class Container, class F>
	    F run_for_each(Container&& container, F&& fun) {
		for (auto& e: container)
		    run([&fun,&e](){
			    fun(e);
			});
		return std::forward<F>(fun);
	    }

	};





	/**
	 * A homogenous thread pool with template parameters
	 * predefining the constructor parameters.
	 */
	template<class Function,
		 int default_thread_count = -1,
		 std::size_t default_queue_size = 0,
		 std::size_t default_maxpart = 1>
	class HomogenousThreadPoolTemplate
	    : public HomogenousThreadPool<Function> {

	public:

	    explicit
	    HomogenousThreadPoolTemplate(int thread_count = default_thread_count,
					 std::size_t queue_size = default_queue_size,
					 std::size_t maxpart = default_maxpart)
		:  HomogenousThreadPool<Function>(thread_count, queue_size, maxpart) {
	    }
	};





    } // End of namespace impl

} // End of namespace ThreadPoolImpl

#endif // !defined(THREADPOOL_IMPL_THREADPOOL_IMPL_HOMOGENOUS_H)
