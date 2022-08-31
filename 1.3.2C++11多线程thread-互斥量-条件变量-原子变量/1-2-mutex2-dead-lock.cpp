//死锁范例1-2-mutex2-dead-lock
#include <iostream>
#include <thread>
#include <mutex>

struct Complex
{
    std::mutex mutex;
    int i;

    Complex() : i(0){}

    void mul(int x)
    {
        std::lock_guard<std::mutex> lock(mutex);
        i *= x;
    }

    void div(int x)
    {
        std::lock_guard<std::mutex> lock(mutex);
        i /= x;
    }

    void both(int x, int y)
    {
        //lock_guard 构造函数加锁， 析构函数释放锁
        std::lock_guard<std::mutex> lock(mutex);
        mul(x); // 获取不了锁
        div(y);
    }

    void init()
    {
        //lock_guard 构造函数加锁， 析构函数释放锁
        std::lock_guard<std::mutex> lock(mutex);
        sub_init();
    }
    void sub_init()
    {
        std::lock_guard<std::mutex> lock(mutex);
    }
};

int main(void)
{
    Complex complex;

    complex.both(32, 23);
    std::cout << "main finish\n";
    return 0;
}
