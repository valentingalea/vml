/// @file examples/threadpool_helloworld_future.cpp
/** @example threadpool_helloworld_future.cpp
    ThreadPool version of helloworld with std::future */
#include <cassert>
#include <ostream>
#include <iostream>
#include <string>
// Include header for ThreadPool
#include "threadpool/threadpool.h"

int main() {
      
    // Create a thread pool
    threadpool::ThreadPool pool;

     // Run a function returning a result
    auto f = pool.run([] () -> std::string { return "Hello world!"; });

    // ... do some other things in parallel to the thread pool.
    
    // Wait for the function to finish and get the result
    std::string s = f.get();
    std::cout << s << std::endl;

}
