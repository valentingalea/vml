ThreadPool for C++11
====================

Contents
--------
- [Introduction](#Introduction)
- [Installation](#Installation)
- [Usage](#Usage)
- [parallel::for_each()](#parallel_for_each)
- [parallel::transform()](#parallel_transform)
- [ThreadPool](#threadpool0)
- [Iterators from functions](#Iterators)
- [Exceptions](#Exceptions)
- [Performance](#Performance)
- [Creating a Library](#Library)
- [Single-threaded version of threadpool API](#Singlethreaded)
- [Compatibility](#Compatibility)
- [License](#License)
- [Implementation](@ref Implementation)




Introduction <a name="Introduction"></a>
------------

This is a threadpool for C++11.

C++11 comes with support for multithreading, but using it can be
difficult.  This thread pool provides easy-to-use interfaces for
parallelization of C++ programs.

Running parts of the program in parallel is only possible if the
computations in these parts are independent.

The trivial case is processing a large array where the computations
for each element of the data structure do not depend on each other. A
parallel version [parallel::for_each()](#parallel_for_each) of the
standard library algorithm [std::for_each()][] allows the
parallelization without caring about threads.

Sometimes the computations may not be so independent. They may be able
to execute independently up to a point, but then they may need to
store their results in a synchronised way. A parallel version
[parallel::transform()](#parallel_transform) of the standard library
algorithm [std::transform()][] runs tasks from an input iterator in
parallel, but writes the results in sequential order to an output
iterator.

To run arbitrary free functions, you must [create a ThreadPool and
run() tasks in it](#threadpool0).



Installation <a name="Installation"></a>
------------

Unpack the distribution to a directory on your local machine. You can
include the proper header in subdirectory `include/threadpool` from
your program. To make inclusion of the headers easier, it is
recommended to add the subdirectory `include` to the include file
search path of the compiler.  This is commonly achieved with the
option `-I/path/to/threadpool/include` (assuming that the threadpool
distribution has been upacked to directory
`/path/to/threadpool`). Then you can include the thread pool headers
through their standard names `"threadpool/xxx.h"`.



Usage <a name="Usage"></a>
-----

In order to use the thread pool, the proper header has to be
included. These are:

~~~{.cpp}
    #include "/path/to/threadpool/include/threadpool/parallel_for_each.h"
    #include "/path/to/threadpool/include/threadpool/parallel_transform.h"
    #include "/path/to/threadpool/include/threadpool/threadpool.h"
    #include "/path/to/threadpool/include/threadpool/make_iterator.h"
~~~

If the threadpool include directory `/path/to/threadpool/include` has
been added to the include file search path of the compiler, e.g. using
the compiler option `-I/path/to/threadpool/include`, this reduces to:

~~~{.cpp}
    #include "threadpool/parallel_for_each.h"
    #include "threadpool/parallel_transform.h"
    #include "threadpool/threadpool.h"
    #include "threadpool/make_iterator.h"
~~~

The thread pool objects and functions are exported through namespace
`threadpool` with sub-namespace `parallel`.  In the examples here I
always use the fully qualified names. They are:

~~~{.cpp}
    threadpool::parallel::for_each()  // Parallel version of std::for_each()
    threadpool::parallel::transform() // Parallel version of std::transform()
    threadpool::ThreadPool            // ThreadPool class
    threadpool::make_function_input_range()
    threadpool::make_function_output_iterator()
~~~

The thread pool is pre-configured for header-only use. This means:
Just include the proper header and you are done. In order to reduce
space overhead and compilation time, a precompiled library can be
used. See section [Creating a Library](#Library).

You may need to link the thread pool against a thread library. For
example on Linux systems this would be the library `-lpthread`. In the
compiler / linker command line, `-lpthread` must come at the end,
behind your source and object files and even behind the threadpool
library (if you use the threadpool library). If you forget to link
against `-lpthread`, you may get no warning at compilation and link
time but the program may crash. So the right compiler command line
would be:

    g++ -std=c++11 -I/path/to/threadpool/include \
        yourprograms.cpp /path/to/threadpool/lib/libthreadpool.a -lpthread



Parallel for_each() <a name="parallel_for_each"></a>
-------------------

The C++ standard library contains an algorithm std::for_each(). The
threadpool implementation parallel::for_each() uses the same function
call interface:

~~~{.cpp}
    Function
    parallel::for_each(InputIterator first, InputIterator last, Function fun);
~~~

The elements to be processed are delimited by the two iterators `first`
(the first element of the range to be processed) and `last` (the element
behind the last element processed). The function `fun` should accept one
parameter, the element to be processed. In C++11, lambda functions can
be used.

Example usage:

~~~{.cpp}
    // Include header for parallel::for_each()
    #include "threadpool/parallel_for_each.h"
    
    // Create a vector of ints from 1 to 9
    std::vector<int> v({1,2,3,4,5,6,7,8,9});
    
    // Multiply each element of the vector with 3
    threadpool::parallel::for_each(v.begin(), v.end(),
                                   [] (int& e) { e *= 3; });
~~~

The funny expression in the last line is the function. C++11's lambda
functions are handy for writing terse but readable small functions
just at the point of call. If you have not seen them:

- The brackets `[]` are the lambda function introducer and the capture
  clause. Between the brackets you can specify which local variables
  of the enclosing function shall be visible inside the lambda
  function.

- The parentheses `(int& e)` contain the normal function parameter
  specification. This function takes an int parameter by
  reference. Taking parameters by reference is useful if you want to
  change the element.

- The braces `{ e *= 3; }` contain the code to be executed. This
  function multiplies each element of the vector with 3.

In the example the lambda function is executed in parallel for all
elements of the vector.

If a whole container shall be processed, parallel::for_each() can be
called with a container instead of the two iterators:

~~~{.cpp}
    Function threadpool::parallel::for_each(Container cnt, Function fun);
~~~

In the example above, we could also have written:

~~~{.cpp}
    // Multiply each element of the vector with 3
    threadpool::parallel::for_each(v, [] (int& e) { e *= 3; });
~~~

If you have several vectors which you need to process together, you
can use the function parallel::for_each() with an integer range and
index the vectors directly:

~~~{.cpp}
    // Create a vector of ints from 1 to 9
    std::vector<int> vi({1,2,3,4,5,6,7,8,9});
    
    // Create a second vector for the results of our computation
    std::vector<int> vo(vi.size());
    
    // Multiply each element of vi with 3, store the result in vo
    threadpool::parallel::for_each(0, vi.size(),
                        [&vi, &vo] (int i) { vo[i] = vi[i] * 3; });
~~~

With the capture clause `[&vi, &vo]` the two vectors are made visible
inside the lambda function. The function now takes an `int` as
parameter, which is incremented by parallel::for_each() from 0 to
`vi.size()-1`, and the lambda function is called with each number in
parallel. The lambda function uses this integer parameter as index
into the two vectors.



Parallel transform() <a name="parallel_transform"></a>
--------------------

The C++ standard library contains an algorithms std::transform(). The
threadpool implementation parallel::transform() uses the same function
call interface:

~~~{.cpp}
    OutputIterator
    threadpool::parallel::transform(InputIterator first, InputIterator last,
                                    OutputIterator result, UnaryOperation op);
~~~

The elements to be processed are delimited by the two iterators
`first` (the first element of the range to be processed) and `last`
(the element behind the last element processed). The function `op`
should accept one parameter, the element to be processed. In C++11,
lambda functions can be used. The function should return a result
which is written through the output iterator `result`.

The interesting thing compared to parallel::for_each() is that, while
the function `op` is called concurrently, the results are written to
the output iterator `result` synchronized and in the order in which
the elements occurred in the input sequence.

Example usage:

~~~{.cpp}
    // Include header for parallel::transform()
    #include "threadpool/parallel_transform.h"
    
    // Create a vector of ints from 1 to 9 and a list for the results
    std::vector<int> v({1,2,3,4,5,6,7,8,9});
    std::list<int> l;
    
    // Multiply each element of the vector with 3, push results into list l
    threadpool::parallel::transform(v.begin(), v.end(),
                                    back_inserter(l),
                                    [] (int& e) -> int { return e * 3; });
~~~

For a description of lambda functions see above in the section about
parallel::for_each(). The return value of a lambda function is
specified between the parentheses with the parameter specification and
the body of the function in braces. It is preceded by `->`.

In the example, the lambda function is called in parallel for all elements
from the input sequence, and the values returned from the function are
pushed into the list `l` in the proper sequence.

If a whole container shall be processed, parallel::transform() can be
called with a container instead of the two iterators:

~~~{.cpp}
    OutputIterator
    threadpool::parallel::transform(Container cnt,
                                    OutputIterator result, UnaryOperation op);
~~~

In the example above, we could also have written:

~~~{.cpp}
    // Multiply each element of the vector with 3, push results into list l
    threadpool::parallel::transform(v, back_inserter(l),
                                    [] (int& e) -> int { return e * 3; });
~~~

If you have several vectors which you need to process together, you
can use the function parallel::transform() with an integer range and
index the vectors directly:

~~~{.cpp}
    // Include header for parallel::transform()
    #include "threadpool/parallel_transform.h"
    
    // Create two vector of ints from 1 to 9 and from 9 to 1
    std::vector<int> v1({1,2,3,4,5,6,7,8,9});
    std::vector<int> v2({9,8,7,6,5,4,3,2,1});
    std::list<int> l;
    
    // Multiply matching elements from v1 and v2, push results into list l
    threadpool::parallel::transform(0, v1.size(),
                                    back_inserter(l),
                                    [&v1, &v2] (int i) -> int {
                                         return v1[i] * v2[i];
                                    });
~~~

With the capture clause `[&v1, &v2]` the two vectors are made visible
inside the lambda function. The function now takes an `int` as
parameter, which is incremented by parallel::transform() from 0 to
`v1.size()-1`, and the lambda function is called with each number in
parallel. The lambda function uses this integer parameter as index
into the two vectors and returns the result of the
multiplication. The results are pushed into the list `l` in sequence.




ThreadPool <a name="threadpool0"></a>
----------

The parallel agorithms create a thread pool internally. In order to
execute arbitrary functions in parallel, a thread pool must be
instantiated:

~~~{.cpp}
    // Include header for ThreadPool
    #include "threadpool/threadpool.h"
    
    // Create a thread pool
    threadpool::ThreadPool pool;
    
    // Run a function in the thread pool
    for (int i = 0; i < 10; ++i)
        pool.run([]{ std::cout << "Hello world!" << std::endl; });
    
    // Wait for all queued functions to finish and the pool to become empty
    pool.wait();
~~~

Use the ThreadPool's member function run() to queue a task for
execution. All tasks queued for execution will run in parallel. If at
a later time you need the results, use the ThreadPool's member
function wait(). It will wait until all tasks have finished. Later on
you may want to run() more tasks in the thread pool.

Note that it is generally a bad idea to modify any objects
concurrently from multiple threads, for example to try to append
things to a container concurrently. The standard output streams are an
exception, the standard does explicitly guarantee that the integrity
of the stream objects is preserved. of course the output might be
mangled.

Functions to be queued with run() must be callable with no
arguments. In the example above, the function had the signature
`void()` and did not return something. What if you need to return a
result from the function?

If the function returns a result, run() returns a `std::future`. The
member function get() of the `future` waits for the function to finish
(unless it has already finished) and returns the result. Assuming the
thread pool created above does still exist:

~~~{.cpp}
    // Run a function returning a result
    auto f = pool.run([] () -> std::string { return "Hello world!"; });

    // ... do some other things in parallel to the thread pool.
    
    // Wait for the function to finish and get the result
    std::string s = f.get();
    std::cout << s << std::endl;
~~~

The lambda version returns an `std::string` object. It is queued using
run(). Then the main thread can do some other things. Once it needs
the result from the task, it calls the member function get() of the
`std::future` returned by run(). Note that get() will not only return
the return value from the function but will also rethrow any
exceptions the function might have thrown.

Since it is possible to execute arbitrary functions, it is easy to
process elements of an array in parallel like parallel::for_each():

~~~{.cpp}
    // Create a vector of ints from 1 to 9
    std::vector<int> v({1,2,3,4,5,6,7,8,9});
    
    // Multiply each element of the vector with 3
    for (int &e: v) 
    	pool.run([&e] { e *= 3; });
    
    // Wait for all tasks to finish
    pool.wait();
~~~

The capture clause `[&e]` gives the lambda function access to the
element used in the `for` statement. The element is passed by
reference. This is right in this case because the elements of the
vector referenced by `e` will continue to exist until after wait() is
called. If wait() would not be called and the function defining the
vector `v` would be left instead, the tasks still running would access
invalid memory locations.  Because the function is executed
asynchronously, it is very important to use the proper capture
clause. The following for example would be invalid:

~~~{.cpp}
    // BROKEN EXAMPLE, WOULD NOT WORK!!!
    
    // Create a vector of ints from 1 to 9
    std::vector<int> v({1,2,3,4,5,6,7,8,9});

    // Multiply each element of the vector with 3
    for (unsigned i = 0; i < v.size(); ++i) 
    	pool.run([&i, &v] { v[i] *= 2; }); // WOULD NOT WORK!!! USES STALE I!!!
~~~

In this broken example the loop index `i` is passed by reference. But
when the lambda function is run later on, `i` will already have been
incremented! Still worse, the `for` loop will have been finished, and
the location of the loop index `i` will not be valid, may even contain
something completely different!

To fix the broken example, you have to pass the loop index by
value. To do this, just leave away the ampersand `&` in front of `i`
in the capture clause:
    
~~~{.cpp}
    // Multiply each element of the vector with 3
    for (unsigned i = 0; i < v.size(); ++i) 
    	pool.run([i, &v] { v[i] *= 2; }); // Ok, loop index i passed by value
~~~

Now the loop index is passed by value, and everything works.



Iterators from functions <a name="Iterators"></a>
------------------------

Sometimes an algorithm might look useful, but the data are not
available in Iterator form. Maybe the values are returned from a
function. Maybe it is possible to write a small Lambda function to
return the values. Or maybe the output values shall be passed off to
the function. In this case you can convert the function to an
iterator. The header `make_iterator.h` contains two functions
make_function_input_range() and make_function_output_iterator() that
turn functions into iterators.



### make_function_input_range() ###


Function

~~~{.cpp}
    <Range>
    threadpool::make_function_input_range(Function fun);
~~~

takes a function `fun` and returns an input iterator `<Range>`. The
type of the range should not specified by the caller, the result of
make_function_input_range() should instead be assigned to an `auto`
variable, like here:

~~~{.cpp}
    // Include header for make_function_input_range()
    #include "threadpool/make_iterator.h"
    
    // Create input iterator returning numbers from 1 to 10
    auto it = threadpool::make_function_input_range(
        []() -> int {
            static int u = 1;
            if (u > 10)
                throw std::out_of_range("");
            return u++;
        });
~~~

In this example an iterator range is created which returns all the
number from 1 to 10. Note that the function must throw an
`std::out_of_range` exception to signal that it has no more
values. The result of make_function_input_range() is assigned to the
variable `it` which is declared as `auto`, so you do not need to care
about its true convoluted type. The variable `it` now has two member
functions begin() and end() which can be passed as `first` and `last`
values to any algorithms from the thread pool or from the standard
library:

~~~{.cpp}
    std::vector<int> a;

    // Multiply each member by 2 and store in vector a.
    threadpool::parallel::transform(it.begin(), it.end(), back_inserter(a),
                                    [](int i) -> int { return 2 * i; });
~~~

Function parallel::tansform() reads values through iterator it.begin(),
which as we have seen above returns the integer numbers from 1 to
10 in sequence. The lambda function multiplies all values with 2 (in parallel),
and the results are written through the back_inserter to the end of
the vector `a`.

Because the range has member functions begin() and end(), it looks
like a container, and we could simply use the container version of
parallel::transform():

~~~{.cpp}
    std::vector<int> a;
    threadpool::parallel::transform(it, back_inserter(a),
                                    [](int i) -> int { return 2 * i; });
~~~

Or the range could be used with the range-based for statement:

~~~{.cpp}
    // Write all numbers to standard output
    for (int i: it)
        std::cout << " " << i;
~~~

The function passed to make_function_input_range() should be callable
with no arguments, and should return one value each time it is
called. When it has no more values, it should throw an exception
`std::out_of_range`.



### make_function_output_iterator() ###


Function

~~~{.cpp}
    <Iterator>
    threadpool::make_function_output_iterator(Function fun);
~~~

takes a function `fun` and returns an output `<Iterator>`. The type of
the iterator should not be specified by the caller, the result of
make_function_output_iterator() should instead be assigned to an
`auto` variable, like here:

~~~{.cpp}
    // Include header for make_function_output_iterator()
    #include "threadpool/make_iterator.h"

    // Create output iterator writing numbers to standard output
    auto ot = threadpool::make_function_output_iterator(
        [] (int i) {
            std::cout << " " << i;
        });
~~~

In this example an output iterator is created which writes all values
to the standard output. The output iterator can be used like here:

~~~{.cpp}
    std::vector<int> a({1,2,3,4,5,6,7,8,9,10});

    // Multiply number by two and write to standard output
    threadpool::parallel::transform(a, ot,
                                    [](int i) -> int { return 2 * i; });
~~~

Function parallel::transform() takes all numbers from the input
sequence, passes them in parallel to the lambda function which
multiplies the number by 2, and passes them in sequence to the output
iterator `ot`, which writes the numbers to the standard output.



Exceptions <a name="Exceptions"></a>
----------

If a task running under the thread pool throws an exception, the
remaining tasks that have not yet started are flushed and the threads
return. The thread pool ends up unusable but destroyable. On the next
wait(), or at the time the thread pool is destructed, the exception is
re-raised in the thread calling wait() or destroying the thread pool.

If functions returning a result are [run()](#threadpool0), run()
returns an `std::future` that gives access to the return value of the
function. All exceptions from this function are catched by the thread
pool and returned through the `std::future`. See an example in the
[section about run()](#threadpool0).



Performance <a name="Performance"></a>
-----------


### Overhead ###

Performance of the thread pool should not be an issue for long-running
tasks (longer than 1 ms), since the overhead of the thread dispatcher
will be insignificant compared to the work load.  For small tasks, it
is useful to know the overhead per element. On my `Intel(R) Core(TM)
i7-3820 CPU @ 3.60GHz`, I measure the following overhead:

- parallel::for_each() and parallel::transform() with a lambda
  function on a vector of 1 million elements: overhead per element < 1
  ns.

- parallel::transform() on a vector of 1 million elements, returning
  the result through a back_inserter into a list: overhead per element
  < 1 µs.

- ThreadPool lambda execution with run(): overhead < 1 µs.

- ThreadPool lambda execution, result returned via `std::future`:
  overhead < 6 µs.


### Speed Gain ###

On compute-bound tasks, on a four-core system (eight hyperthreads) I
observe speedups of about a factor of five. If the tasks are memory-
or I/O bound, it may not be possible to gain anything, or it may be
possible to get a speed advantage by tuning the number of threads
instantiated in the thread pool.

The default number of threads used by the thread pool is the number of
hardware processors as returned by
std::thread:hardware_concurrency(). On compute-bound tasks, this
default should work fine. On I/O- bound tasks, it may be useful to use
more threads.

With `ThreadPool`, the number of threads to instantiate is specified
as an argument to the constructor. For example

~~~{.cpp}
    threadpool::ThreadPool pool(100);
~~~

creates a thread pool with hundred threads. Evidently, if this number
is larger than the number of hardware processors not all of these 100
threads may be able to execute at every one time. But maybe they can
all wait on some I/O.

With the parallel algorithms parallel::for_each() and
parallel::transform(), the number of threads can be specified using a
template argument. For example

~~~{.cpp}
    threadpool::parallel::for_each<100>(v, [] (int& e) { e *= 2; });
~~~

multiplies all integer elements of the container `v` with 2, using 100
threads.



Creating a Library <a name="Library"></a>
------------------

The thread pool is pre configured for header-only use. This means:
Just include the proper header and you are done. In order to reduce
space overhead and compilation time, a precompiled library can be
used.

To create the library, the two C++ source files `threadpool.cpp` and
`threadpool_generic.cpp` in directory `lib` of the distribution must
be compiled. Under Linux, just run `make`. Before compiling, you may
want to select the compiler to use: Uncomment to proper *CXX=* - line
in the toplevel Makefile.template.  Running `make` should create a
library `lib/libthreadpool.a`, which has to be linked to the programs.

In order to make the header use the library, you must open the header
`threadpool/threadpool_config.h` with an editor and change the preprocessor
symbol `THREADPOOL_USE_LIBRARY` from 0 to 1. The next time a program
is compiled, the library will be used. You can check that the library
is used as intended by omitting the library when linking.  Linking
should fail with missing externals.



Single-threaded Version of the Threadpool API <a name="Singlethreaded"></a>
---------------------------------------------

For use on systems without multithreading support, and for use during
debugging, there is a single-threaded version of the thread pool API
available. The headers can be found in subdirectory `singlethreaded`
of the main threadpool include directory:

~~~{.cpp}
    #include "threadpool/singlethreaded/parallel_for_each.h"
    #include "threadpool/singlethreaded/parallel_transform.h"
    #include "threadpool/singlethreaded/threadpool.h"
~~~

The thread pool functions and class of the singlethreaded version
are exported through namespace `threadpool::singlethreaded` with
sub-namespace `parallel`. They are:

~~~{.cpp}
    threadpool::singlethreaded::parallel::for_each()
    threadpool::singlethreaded::parallel::transform()
    threadpool::singlethreaded::ThreadPool
~~~

Since these names mirror the names of their multithreaded cousins,
switching between multithreaded and singlethreaded version can be
accomplished using a namespace alias definition, for example like
this:

~~~{.cpp}
    namespace TP = threadpool;                    // For multithreaded use
    // namespace TP = threadpool::singlethreaded; // For singlethreaded use

    TP::ThreadPool pool;              // Use single-or multithreaded version
    TP::parallel::foreach();          // depending on namespace alias TP
~~~




Compatibility <a name="Compatibility"></a>
-------------

The threadpool has been tested with:

- GCC 4.7 and 4.8 on Linux
- Visual Studio Express 2013 for Windows Desktop with November 2013 CTP
- Intel C++ 14.0.2 on Linux




License <a name="License"></a>
-------

Copyright (c) 2014 Ruediger Helsch; All rights reserved

Use this software however you want. No warranty whatsoever.



[std::for_each()]: http://en.cppreference.com/w/cpp/algorithm/for_each
[std::transform()]: http://en.cppreference.com/w/cpp/algorithm/transform
