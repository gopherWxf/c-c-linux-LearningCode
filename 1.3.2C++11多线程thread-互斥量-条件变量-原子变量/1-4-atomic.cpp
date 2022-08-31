// atomic::load/store example
#include <iostream>       // std::cout
#include <atomic>         // std::atomic, std::memory_order_relaxed
#include <thread>         // std::thread

//std::atomic<int> count = 0;//错误初始化
std::atomic<int> count(0); // 准确初始化

void set_count(int x)
{
    std::cout << "set_count:" << x << std::endl;
    count.store(x, std::memory_order_relaxed);     // set value atomically
}

void print_count()
{
    int x;
    do {
        x = count.load(std::memory_order_relaxed);  // get value atomically
    } while (x==0);
    std::cout << "count: " << x << '\n';
}

int main ()
{
    std::thread t1 (print_count);
    std::thread t2 (set_count, 10);
    t1.join();
    t2.join();
    std::cout << "main finish\n";
    return 0;
}
