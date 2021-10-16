/// @file examples/threadpool_helloworld.cpp
/** @example threadpool_helloworld.cpp
    ThreadPool version of helloworld */
#include <cassert>
#include <ostream>
#include <iostream>
// Include header for ThreadPool
#include "threadpool/threadpool.h"

int main() {
      
    // Create a thread pool
    threadpool::ThreadPool pool;
    
    // Run a function in the thread pool
    for (int i = 0; i < 10; ++i)
        pool.run([]{ std::cout << "Hello world!" << std::endl; });
    
    // Wait for all queued functions to finish and the pool to become empty
    pool.wait();

    // Expect mangled output, but all characters from 10 hello word. 
}
