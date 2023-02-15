#include <iostream>

#include "threadpool.h"

int main() {
    ThreadPool threadpool;
    threadpool.Start(5);
    std::future future = threadpool.Submit([](){
        std::cout << "hello, world" << std::endl;
    });
    future.get();

    return 0;
}
