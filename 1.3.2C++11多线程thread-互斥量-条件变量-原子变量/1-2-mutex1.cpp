//1-2-mutex1
#include <iostream>       // std::cout
#include <thread>         // std::thread
#include <mutex>          // std::mutex

volatile int counter(0); // non-atomic counter
std::mutex mtx;           // locks access to counter

void increases_10k()
{
    for (int i=0; i<10000; ++i) {
        // 1. 使用try_lock的情况
//        if (mtx.try_lock()) {   // only increase if currently not locked:
//            ++counter;
//            mtx.unlock();
//        }
        // 2. 使用lock的情况
                {
                    mtx.lock();
                    ++counter;
                    mtx.unlock();
                }
    }
}

int main()
{
    std::thread threads[10];
    for (int i=0; i<10; ++i)
        threads[i] = std::thread(increases_10k);

    for (auto& th : threads) th.join();
    std::cout << " successful increases of the counter "  << counter << std::endl;

    return 0;
}
