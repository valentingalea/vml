/// @file examples/threadpool_pass_by_value.cpp
/** @example threadpool_pass_by_value.cpp
    Know when to pass lambda arguments by value or by reference */
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
    for (unsigned i = 0; i < v.size(); ++i) 
    	pool.run([i, &v] { v[i] *= 2; }); // Ok, loop index i passed by value
   
    // Wait for all tasks to finish
    pool.wait();

    for (int i = 0; i < (int)v.size(); ++i)
	assert(v[i] == 2 * (i + 1));
    std::cout << "PASS" << std::endl;

}
