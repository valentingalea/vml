/// @file examples/make_function_input_range_for_loop.cpp
#include <cassert>
#include <ostream>
#include <iostream>
#include <vector>
// Include header for make_function_output_iterator()
#include "threadpool/make_iterator.h"
// Include header for parallel::transform()
#include "threadpool/parallel_transform.h"

int main() {
    
    // Create output iterator writing numbers to standard output
    auto ot = threadpool::make_function_output_iterator(
        [] (int i) {
            std::cout << " " << i;
        });
 
    std::vector<int> a({1,2,3,4,5,6,7,8,9,10});

    // Multiply number by two and write to standard output
    threadpool::parallel::transform(a, ot,
                                    [](int i) -> int { return 2 * i; });

    // Though computation occurs in parallel, results are written in sequence.
    std::cout << " PASS" << std::endl;
}
