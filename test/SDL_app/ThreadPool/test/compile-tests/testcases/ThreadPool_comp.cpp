#include <ostream>
#include <iostream>
#include "threadpool/threadpool.h"

int main() {
    threadpool::ThreadPool pool;
    pool.run([]{std::cerr << "PASS" << std::endl;});
}
