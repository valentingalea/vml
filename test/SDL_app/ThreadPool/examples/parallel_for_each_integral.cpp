/// @file examples/parallel_for_each_integral.cpp
/** @example parallel_for_each_integral.cpp
    Using parallel::for_each() with integer ranges */
#include <cassert>
#include <ostream>
#include <iostream>
#include <vector>
// Include header for parallel::for_each()
#include "threadpool/parallel_for_each.h"

int main() {
    
    // Create a vector of ints from 1 to 9
    std::vector<int> vi({1,2,3,4,5,6,7,8,9});
    
    // Create a second vector for the results of our computation
    std::vector<int> vo(vi.size());
    
    // Multiply each element of vi with 3, store the result in vo
    threadpool::parallel::for_each(0, vi.size(),
				   [&vi, &vo] (int i) { vo[i] = vi[i] * 3; });

    for (int i = 0; i < (int)vo.size(); ++i)
	assert(vo[i] == 3 * vi[i]);
    std::cout << "PASS" << std::endl;

}
