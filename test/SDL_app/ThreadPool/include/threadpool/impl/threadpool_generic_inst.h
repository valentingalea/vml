/** @file threadpool/impl/threadpool_generic_inst.h
 *
 * Threadpool for C++11, instantiation of generic thread pool
 *
 * @copyright	2014 Ruediger Helsch, Ruediger.Helsch@t-online.de
 * @license	All rights reserved. Use however you want. No warranty at all.
 * $Revision: 2.0 $
 * $Date: 2014/05/14 16:56:58 $
 */

#include "threadpool_generic_impl.h"
#include "threadpool_generic.h"

#ifndef THREADPOOL_IMPL_THREADPOOL_GENERIC_INST_H
#define THREADPOOL_IMPL_THREADPOOL_GENERIC_INST_H

namespace ThreadPoolImpl {

    namespace impl {

	template<int I>
	GenericThreadPoolTmpl<I>::GenericThreadPoolTmpl(GenericThreadPoolQueue& queue,
							int thread_count)
	    : pimpl(new GenericThreadPoolImpl<GenericThreadPoolQueue>(queue, thread_count))
	{ }

	template<int I>
	void GenericThreadPoolTmpl<I>::help(bool return_if_idle) {
	    pimpl->help(return_if_idle);
	}

	template<int I>
	void GenericThreadPoolTmpl<I>::rethrow_exception() {
	    pimpl->rethrow_exception();
	}

	template<int I>
	void GenericThreadPoolTmpl<I>::join() {
	    pimpl->join();
	}

	template<int I>
	GenericThreadPoolTmpl<I>::~GenericThreadPoolTmpl() {
	}

	template<int I>
	unsigned int GenericThreadPoolTmpl<I>::hardware_concurrency(int c) {
	    return GenericThreadPoolImpl<GenericThreadPoolQueue>::hardware_concurrency(c);
	}

	template<int I>
	unsigned int GenericThreadPoolTmpl<I>::determine_thread_count(int c) {
	    return GenericThreadPoolImpl<GenericThreadPoolQueue>::determine_thread_count(c);
	}

	template<int I>
	bool GenericThreadPoolTmpl<I>::ignore_thread_pool_exceptions(bool i) {
	    return GenericThreadPoolImpl<GenericThreadPoolQueue>::ignore_thread_pool_exceptions(i);
	}

    } // End of namespace impl

} // End of namespace ThreadPoolImpl

#endif // THREADPOOL_IMPL_THREADPOOL_GENERIC_INST_H
