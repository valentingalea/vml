/** @file threadpool/impl/threadpool_inst.h
 *
 * Threadpool for C++11, library version of virtual thread pool
 *
 * @copyright	2014 Ruediger Helsch, Ruediger.Helsch@t-online.de
 * @license	All rights reserved. Use however you want. No warranty at all.
 * $Revision: 2.0 $
 * $Date: 2014/05/14 16:56:58 $
 */
#ifndef THREADPOOL_IMPL_THREADPOOL_INST_H
#define THREADPOOL_IMPL_THREADPOOL_INST_H

#include "../threadpool.h"
#include "threadpool_impl.h"

namespace ThreadPoolImpl {

    namespace impl {



	template<int I>
	VirtualThreadPool<I>::VirtualThreadPool(int thread_count,
						std::size_t queue_size,
						std::size_t maxpart)
	    : pimpl(new VirtualThreadPoolImpl<>(thread_count, queue_size, maxpart))
	{  }


	template<int I>
	void VirtualThreadPool<I>::run(std::unique_ptr<Task>&& t) {
	    pimpl->run(std::move(t));
	}

	template<int I>
	void VirtualThreadPool<I>::run(Task* t) {
	    pimpl->run(t);
	}

	template<int I>
	void VirtualThreadPool<I>::wait() {
	    pimpl->wait();
	}

	template<int I>
	void VirtualThreadPool<I>::join() {
	    pimpl->join();
	}

	template<int I>
	VirtualThreadPool<I>::~VirtualThreadPool() {
	}



    } // End of namespace impl

} // End of namespace ThreadPoolImpl

#endif // !THREADPOOL_IMPL_THREADPOOL_INST_H
