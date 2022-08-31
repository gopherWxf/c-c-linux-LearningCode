#ifndef SYNC_QUEUE_H
#define SYNC_QUEUE_H

#include<list>
#include<mutex>
#include<thread>
#include<condition_variable>
#include <iostream>

template<typename T>
class SyncQueue {
private:
    bool IsFull() const {
        return _queue.size() == _maxSize;
    }

    bool IsEmpty() const {
        return _queue.empty();
    }

public:
    SyncQueue(int maxSize) : _maxSize(maxSize) {
    }

    void Put(const T &x) {
        std::lock_guard <std::mutex> locker(_mutex);

        while (IsFull()) {
            std::cout << "full wait... size " << _queue.size() << std::endl;
            _notFull.wait(_mutex);
        }

        _queue.push_back(x);
        _notEmpty.notify_one();
    }

    void Take(T &x) {
        std::lock_guard <std::mutex> locker(_mutex);

        while (IsEmpty()) {
            std::cout << "empty wait.." << std::endl;
            _notEmpty.wait(_mutex);
        }

        x = _queue.front();
        _queue.pop_front();
        _notFull.notify_one();
    }

    bool Empty() {
        std::lock_guard <std::mutex> locker(_mutex);
        return _queue.empty();
    }

    bool Full() {
        std::lock_guard <std::mutex> locker(_mutex);
        return _queue.size() == _maxSize;
    }

    size_t Size() {
        std::lock_guard <std::mutex> locker(_mutex);
        return _queue.size();
    }

    int Count() {
        return _queue.size();
    }

private:
    std::list <T> _queue;                  //缓冲区
    std::mutex _mutex;                    //互斥量和条件变量结合起来使用
    std::condition_variable_any _notEmpty;//不为空的条件变量
    std::condition_variable_any _notFull; //没有满的条件变量
    int _maxSize;                         //同步队列最大的size
};

#endif // SYNC_QUEUE_H
