/// @file examples/threadpool_for_each.cpp
/** @example threadpool_for_each.cpp
    ThreadPool version of running a function on each element of an array */
#include <cassert>
#include <ostream>
#include <iostream>
#include <vector>
// Include header for ThreadPool
#include "threadpool/threadpool.h"

int main() {
      
    // Create a thread pool
    threadpool::ThreadPool pool;

    // Create a vector of ints from 1 to 9
    std::vector<int> v({1,2,3,4,5,6,7,8,9});
    
    // Multiply each element of the vector with 3
    for (int &e: v) 
    	pool.run([&e] { e *= 3; });
    
    // Wait for all tasks to finish
    pool.wait();

    for (int i = 0; i < (int)v.size(); ++i)
	assert(v[i] == 3 * (i + 1));
    std::cout << "PASS" << std::endl;

}
