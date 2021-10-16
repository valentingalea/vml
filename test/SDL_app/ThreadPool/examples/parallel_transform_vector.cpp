/// @file examples/parallel_transform_vector.cpp
/** @example parallel_transform_vector.cpp
    Using parallel::transform() on entire containers */
#include <cassert>
#include <ostream>
#include <iostream>
#include <vector>
#include <list>
// Include header for parallel::transform()
#include "threadpool/parallel_transform.h"

int main() {
      
    // Create a vector of ints from 1 to 9 and a list for the results
    std::vector<int> v({1,2,3,4,5,6,7,8,9});
    std::list<int> l;
    
    // Multiply each element of the vector with 3, push results into list l
    threadpool::parallel::transform(v, back_inserter(l),
                                    [] (int& e) -> int { return e * 3; });
 
    auto lp = l.begin();
    for (int i = 0; i < (int)v.size(); ++i)
	assert(*lp++ == 3 * v[i]);
    std::cout << "PASS" << std::endl;

}
