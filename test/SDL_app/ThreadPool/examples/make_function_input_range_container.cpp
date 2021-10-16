/// @file examples/make_function_input_range_container.cpp
/** @example make_function_input_range_container.cpp
    Creating an input operator range from a function, used as container */
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
 
    // Multiply each member by 2 and store in vector a.
    std::vector<int> a;
    threadpool::parallel::transform(it, back_inserter(a),
                                    [](int i) -> int { return 2 * i; });

    assert(a.size() == 10);
    for (int i = 0; i < 10; ++i)
	assert(a[i] == 2 * (i + 1));
    std::cout << "PASS" << std::endl;

}
