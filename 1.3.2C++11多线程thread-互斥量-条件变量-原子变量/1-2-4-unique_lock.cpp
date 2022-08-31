#include <iostream>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unistd.h>

std::deque<int> q;
std::mutex mu;
std::condition_variable cond;
int count = 0;

void fun1() {
    while (true) {
        {
        std::unique_lock<std::mutex> locker(mu); // 能否换成lock_guard  lock
        std::cout << "fun1 lock\n";
        q.push_front(count++);
//        locker.unlock();        // 这里是不是必须的？ lock_guard是没有手动释放锁的 unlock

            cond.notify_one();  //condition_variable 条件和Linux条件变量一样的
        }
        sleep(1);
    }
}

void fun2() {
    while (true) {
        std::unique_lock<std::mutex> locker(mu);
        std::cout << "fun2 lock\n";
        std::cout << "fun2 wait into\n";
        cond.wait(locker, [](){return !q.empty();});
        std::cout << "fun2 wait leave\n";
        auto data = q.back();
        q.pop_back();
//        locker.unlock(); // 这里是不是必须的？
        std::cout << "thread2 get value form thread1: " << data << std::endl;
    }
}
int main() {
    std::thread t1(fun1);
    std::thread t2(fun2);
    t1.join();
    t2.join();
    return 0;
}
