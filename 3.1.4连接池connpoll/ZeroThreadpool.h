//zero_threadpool.h
#ifndef ZERO_THREADPOOL_H
#define ZERO_THREADPOOL_H

#include <future>
#include <functional>
#include <iostream>
#include <queue>
#include <mutex>
#include <memory>

#ifdef WIN32
#include <windows.h>
#else

#include <sys/time.h>

#endif
using namespace std;


void getNow(timeval *tv);

int64_t getNowMs();

#define TNOW      getNow()
#define TNOWMS    getNowMs()

/////////////////////////////////////////////////
/**
 * @file zero_thread_pool.h
 * @brief 线程池类,采用c++11来实现了，
 * 使用说明:
 * ZERO_ThreadPool tpool;
 * tpool.init(5);   //初始化线程池线程数
 * //启动线程方式
 * tpool.start();
 * //将任务丢到线程池中
 * tpool.exec(testFunction, 10);    //参数和start相同
 * //等待线程池结束
 * tpool.waitForAllDone(1000);      //参数<0时, 表示无限等待(注意有人调用stop也会推出)
 * //此时: 外部需要结束线程池是调用
 * tpool.stop();
 * 注意:
 * ZERO_ThreadPool::exec执行任务返回的是个future, 因此可以通过future异步获取结果, 比如:
 * int testInt(int i)
 * {
 *     return i;
 * }
 * auto f = tpool.exec(testInt, 5);
 * cout << f.get() << endl;   //当testInt在线程池中执行后, f.get()会返回数值5
 *
 * class Test
 * {
 * public:
 *     int test(int i);
 * };
 * Test t;
 * auto f = tpool.exec(std::bind(&Test::test, &t, std::placeholders::_1), 10);
 * //返回的future对象, 可以检查是否执行
 * cout << f.get() << endl;
 */

class ZERO_ThreadPool {
protected:
    struct TaskFunc {
        TaskFunc(uint64_t expireTime) : _expireTime(expireTime) {}

        std::function<void()> _func;
        int64_t _expireTime = 0;    //超时的绝对时间
    };

    typedef shared_ptr<TaskFunc> TaskFuncPtr;
public:
    /**
    * @brief 构造函数
    *
    */
    ZERO_ThreadPool();

    /**
    * @brief 析构, 会停止所有线程
    */
    virtual ~ZERO_ThreadPool();

    /**
    * @brief 初始化.
    *
    * @param num 工作线程个数
    */
    bool init(size_t num);

    /**
    * @brief 获取线程个数.
    *
    * @return size_t 线程个数
    */
    size_t getThreadNum() {
        std::unique_lock<std::mutex> lock(_mutex);

        return _threads.size();
    }

    /**
    * @brief 获取当前线程池的任务数
    *
    * @return size_t 线程池的任务数
    */
    size_t getJobNum() {
        std::unique_lock<std::mutex> lock(_mutex);
        return _tasks.size();
    }

    /**
    * @brief 停止所有线程, 会等待所有线程结束
    */
    void stop();

    /**
    * @brief 启动所有线程
    */
    bool start(); // 创建线程

    /**
    * @brief 用线程池启用任务(F是function, Args是参数)
    *
    * @param ParentFunctor
    * @param tf
    * @return 返回任务的future对象, 可以通过这个对象来获取返回值
    */
    template<class F, class... Args>
    auto exec(F &&f, Args &&... args) -> std::future<decltype(f(args...))> {
        return exec(0, f, args...);
    }

    /**
    * @brief 用线程池启用任务(F是function, Args是参数)
    *
    * @param 超时时间 ，单位ms (为0时不做超时控制) ；若任务超时，此任务将被丢弃
    * @param bind function
    * @return 返回任务的future对象, 可以通过这个对象来获取返回值
    */
    /*
    template <class F, class... Args>
    它是c++里新增的最强大的特性之一，它对参数进行了高度泛化，它能表示0到任意个数、任意类型的参数
    auto exec(F &&f, Args &&... args) -> std::future<decltype(f(args...))>
    std::future<decltype(f(args...))>：返回future，调用者可以通过future获取返回值
    返回值后置
    */
    template<class F, class... Args>
    auto exec(int64_t timeoutMs, F &&f, Args &&... args) -> std::future<decltype(f(args...))> {
        int64_t expireTime = (timeoutMs == 0 ? 0 : TNOWMS + timeoutMs);  // 获取现在时间
        //定义返回值类型
        using RetType = decltype(f(args...));  // 推导返回值
        // 封装任务
        auto task = std::make_shared<std::packaged_task<RetType()>>(std::bind(std::forward<F>(f),
                                                                              std::forward<Args>(args)...));

        TaskFuncPtr fPtr = std::make_shared<TaskFunc>(expireTime);  // 封装任务指针，设置过期时间
        fPtr->_func = [task]() {  // 具体执行的函数
            (*task)();
        };

        std::unique_lock<std::mutex> lock(_mutex);
        _tasks.push(fPtr);              // 插入任务
        _condition.notify_one();        // 唤醒阻塞的线程，可以考虑只有任务队列为空的情况再去notify

        return task->get_future();;
    }

    /**
    * @brief 等待当前任务队列中, 所有工作全部结束(队列无任务).
    *
    * @param millsecond 等待的时间(ms), -1:永远等待
    * @return           true, 所有工作都处理完毕
    *                   false,超时退出
    */
    bool waitForAllDone(int millsecond = -1);

protected:
    /**
    * @brief 获取任务
    *
    * @return TaskFuncPtr
    */
    bool get(TaskFuncPtr &task);

    /**
    * @brief 线程池是否退出
    */
    bool isTerminate() { return _bTerminate; }

    /**
    * @brief 线程运行态
    */
    void run();

protected:

    /**
    * 任务队列
    */
    queue<TaskFuncPtr> _tasks;

    /**
    * 工作线程
    */
    std::vector<std::thread *> _threads;

    std::mutex _mutex;

    std::condition_variable _condition;

    size_t _threadNum;

    bool _bTerminate;

    std::atomic<int> _atomic{0};
};

#endif // ZERO_THREADPOOL_H
