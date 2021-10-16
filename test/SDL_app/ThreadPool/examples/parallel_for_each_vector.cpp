/// @file examples/parallel_for_each_vector.cpp
/** @example parallel_for_each_vector.cpp
    Using parallel::for_each() on entire containers */
#include <cassert>
#include <ostream>
#include <iostream>
#include <vector>
// Include header for parallel::for_each()
#include "threadpool/parallel_for_each.h"

int main() {
    
    // Create a vector of ints from 1 to 9
    std::vector<int> v({1,2,3,4,5,6,7,8,9});
    
    // Multiply each element of the vector with 3
    threadpool::parallel::for_each(v, [] (int& e) { e *= 3; });

    for (int i = 0; i < (int)v.size(); ++i)
	assert(v[i] == (i + 1) * 3);
    std::cout << "PASS" << std::endl;

}
