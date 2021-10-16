Implementation {#Implementation}
==============


Directory structure
-------------------

- *include*: The headers are in directory `include/threadpool`. Directory
  `include/threadpool` itself contains the header files to be included
  by the user, subdirectory `impl` contains the implementation files,
  and subdirectory "singlethreaded" contains the singlethreaded
  implementation of the threadpool API.

- *lib*: The library source code is in directory `lib`.

- *test*: Directory `test` and its subdirectories contain the test suite.

- *examples*: Directory `examples` containst small self-standing
   example programs.

To create the library, run `make` in the top-level directory.  Run
`make test` to run the test suite, `make examples` to create the
examples. The compiler to use and its flags are configured in
`Makefile.template` in the top-level directory.

The implementation of the thread pool resides in namespace ThreadPoolImpl::impl.
@namespace ThreadPoolImpl::impl



Contents
--------
- [Thread management](#thread_management)
- [Homogenous thread pool](#homogenous)
- [Virtual thread pool](#virtual)


Thread management {#thread_management}
-----------------

The thread creation and management ist implemented in header
threadpool/impl/threadpool_generic_impl.h in class
GenericThreadPoolImpl. This class is parameterized with template
parameter Queue. The queue is responsible for providing the work. When
an instance of class GenericThreadPoolImpl is created, the threads are
created and sent to work at member function work() of template
parameter Queue.

Instead of creating multiple instantiations of class
GenericThreadPoolImpl, a class is derived, which uses the queue by a
virtual interface. The interface is defined in header
threadpool/impl/threadpool_interface_generic.h. It consists of two
pure virtual interface classes:

- class iGenericThreadPoolQueue defines the queue interface,
  consisting of the functions work() and shutdown(). Function work()
  is called by the threads. Function shutdown() should shut the queue
  down and tell the threads to return home to GenericThreadPoolImpl.

- class GenericThreadPoolInterface defines the interface of the thread
  pool, providing the functions help(), rethrow_exception() and
  join(). Function help() is called by any thread wanting to help out
  in the work. Function rethrow_exception() rethrows any exception
  catched by a worker thread. Function join() waits for all threads to
  finish.

The virtual version of the generic thread pool is defined in header
threadpool/impl/threadpool_generic.h. The class GenericThreadPoolTmpl
is derived from the virtual interface GenericThreadPoolInterface and
holds the implementation, an instance of class GenericThreadPoolImpl,
through a pointer. This use of the pimpl idiom isolates the user of
this class from changes in the implementation.

Actually the class GenericThreadPoolTmpl is also not a true class but
a class template, with an integer as template parameter. The template
parameter is never used, and the class template ist only instantiated
with the default template argument 0. The reason for using a class
template is that we need to be able to include it in multiple
independently compiled translation units, and if it would be an
instance of a non-template class, this could fail with multiply
defined symbols. Multiple automatically instantiated instances of the
same template class are guaranteed to be silently removed.

Since we do not intend to specialize class template
GenericThreadPoolTmpl with any other template argument than 0, we
define a typedef of this specialization as GenericThreadPool.

The member functions of class template GenericThreadPoolTmpl are
defined in header threadpool/impl/threadpool_generic_inst.h. This
header is included by threadpool/impl/threadpool_generic.h if the
header-only version of the thread pool is used. If the library version
is used, the implementation is hidden from the compiler, and the class
template instantiation GenericThreadPoolTmpl<> is declared as
extern. This speeds up compilation, since the compiler doesn't need to
look at the internals.

If the library version is used, the program has to be linked to an
instantiation of GenericThreadPoolTmpl<>. This instantiation is
provided in the source file lib/threadpool_generic.cpp.



Homogenous thread pool {#homogenous}
----------------------

The homogenous thread pool runs tasks from a queue of one class of
functions. Class template HomogenousThreadPool is defined in header
threadpool/impl/threadpool_impl_homogenous.h.

The main part of the homogenous thread pool is the queue HQueue, which
pushes tasks onto a queue and passes them to the threads. Internally,
the queue is implemented as an std::vector. A queue could be
implemented more easily as an std::deque, but concurrent access to
both ends of an std::deque ist not allowed.

For the thread management the monogenous thread pool uses a
GenericThreadPool.

The homogenous thread pool takes the class `Function` as a template
parameter. The only conditions for the template argument Function are
that it must be callable without arguments, and that it must be
moveable.

The homogenous thread pool could be instantiated with a specific
lambda function as template argument `Function`, but since all lambda
functions have their own and different type, it would only be possible
to queue this same lambda function. More useful is an instantiation
with template argument `std::function<void()>`, which allows passing
all functions callable without arguments.



Virtual thread pool {#virtual}
===================

Instead of instantiating the homogenous thread pool with template
argument `std::function<void()>`, we define a thread pool taking the
function by a virtual interface.  This interface is defined in header
threadpool/impl/threadpool_interface_virtual.h.  It consists of the
pure interface class VirtualThreadPool_Task and has only one member
function, the void operator()().

The same header defines the interface to the thread pool
VirtualThreadPoolInterface.  Besides the standard thread pool
interfaces wait() and join(), it has a member function run() which
takes as argument a pointer to a VirtualThreadPool_Task.

The virtual thread pool is implemented in class VirtualThreadPoolImpl
in header threadpool/impl/threadpool_impl.h. Internally a homogenous
thread pool is used. It defines the member type Task, a typedef for
class VirtualThreadPool_Task.  An object of this class must be
allocated by the user and passed to member function run() of the
virtual thread pool.  The virtual thread pool will queue the task for
execution and finally call the task's member function operator()(),
which must do its work and at last delete the task object.

A compiler fire wall for the virtual thread pool is implemented in
class VirtualThreadPool in header threadpool/threadpool.h. Class
VirtualThreadPool uses the pimpl idiom, keeping the implementation
object of class VirtualThreadPoolImpl in a pointer. This means that
when the implementation changes users of class VirtualThreadPool are
not affected and don't need to be recompiled. It also allows the
seamless switch-over between header-only and library
implementation. If the library version is configured, class
VirtualThreadPool is declared as extern template, and the
implementation is hidden from the compiler, which speeds up
compilation. If the header-only version is configured, the
implementation in header threadpool/impl/threadpool_inst.h is
revealed.

If the library version is used, the program has to be linked to an
instantiation of VirtualThreadPool<>. This instantiation is provided
in the source file lib/threadpool.cpp.

Class VirtualThreadPool also defines member function templates run()
for functions with void return type and for functions with nonvoid
return type. Functions with void return type are just executed as-is
by the thread pool. For functions with non-void return type run()
returns an std::future, which can be used to get the return value, and
which also returns an exception which might be thrown by the function.

The virtual thread pool is provided to the user through typedef
threadpool::ThreadPool.
