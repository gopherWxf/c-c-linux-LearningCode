#ifndef ZERO_THREAD_H
#define ZERO_THREAD_H

#include <thread>

class ZERO_Thread {
public:
    ZERO_Thread(); // 构造函数
    virtual ~ZERO_Thread(); // 析构函数
    bool start();

    void stop();

    bool isAlive() const; // 线程是否存活.
    std::thread::id id() { return th_->get_id(); }

    std::thread *getThread() { return th_; }

    void join();  // 等待当前线程结束, 不能在当前线程上调用
    void detach(); //能在当前线程上调用
    static size_t CURRENT_THREADID();

protected:
    void threadEntry();

    virtual void run() = 0; // 运行
protected:
    bool running_; //是否在运行
    std::thread *th_;
};

#endif // ZERO_THREAD_H
