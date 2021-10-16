/// @file examples/parallel_transform_integral.cpp
/** @example parallel_transform_integral.cpp
    Using parallel::transform() with an input range of integers */
#include <cassert>
#include <ostream>
#include <iostream>
#include <vector>
#include <list>
// Include header for parallel::transform()
#include "threadpool/parallel_transform.h"

int main() {
    
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
 
    auto lp = l.begin();
    for (int i = 0; i < (int)v1.size(); ++i)
	assert(*lp++ == v1[i] * v2[i]);
    std::cout << "PASS" << std::endl;

}
