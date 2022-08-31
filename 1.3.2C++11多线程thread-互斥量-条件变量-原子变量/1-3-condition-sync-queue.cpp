#include <iostream>
#include "sync_queue.h"
#include <thread>
#include <iostream>
#include <mutex>

using namespace std;
SyncQueue<int> syncQueue(5);

void PutDatas() {
    for (int i = 0; i < 20; ++i) {
        syncQueue.Put(i);
    }
    std::cout << "PutDatas finish\n";
}

void TakeDatas() {
    int x = 0;

    for (int i = 0; i < 20; ++i) {
        syncQueue.Take(x);
        std::cout << x << std::endl;
    }
    std::cout << "TakeDatas finish\n";
}

int main(void) {
    std::thread t1(PutDatas);  // 生产线程
    std::thread t2(TakeDatas); // 消费线程

    t1.join();
    t2.join();

    std::cout << "main finish\n";
    return 0;
}
