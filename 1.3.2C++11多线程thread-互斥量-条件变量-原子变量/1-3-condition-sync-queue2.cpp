#include <iostream>
#include <thread>
#include <iostream>
#include <mutex>

#include "sync_queue2.h"

using namespace std;
SimpleSyncQueue<int> syncQueue;

void PutDatas() {
    for (int i = 0; i < 20; ++i) {
        syncQueue.Put(888);
    }
}

void TakeDatas() {
    int x = 0;

    for (int i = 0; i < 20; ++i) {
        syncQueue.Take(x);
        std::cout << x << std::endl;
    }
}

int main(void) {
    std::thread t1(PutDatas);
    std::thread t2(TakeDatas);

    t1.join();
    t2.join();

    std::cout << "main finish\n";
    return 0;
}


