/// @file examples/make_function_input_range_for_loop.cpp
/** @example make_function_input_range_for_loop.cpp
    Creating an input operator range from a function, use with for loop */
#include <cassert>
#include <ostream>
#include <iostream>
#include <vector>
// Include header for make_function_input_range()
#include "threadpool/make_iterator.h"
// Include header for parallel::transform()
#include "threadpool/parallel_transform.h"

int main() {
    
    // Create input iterator returning numbers from 1 to 10
    auto it = threadpool::make_function_input_range(
        []() -> int {
            static int u = 1;
            if (u > 10)
                throw std::out_of_range("");
            return u++;
        });
 
    // Write all numbers to standard output
    for (int i: it)
        std::cout << " " << i;

    std::cout << " PASS" << std::endl;
}
