#include "zero_threadpool.h"

ZERO_ThreadPool::ZERO_ThreadPool()
    :  threadNum_(1), bTerminate_(false)
{
}

ZERO_ThreadPool::~ZERO_ThreadPool()
{
    stop();
}

bool ZERO_ThreadPool::init(size_t num)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (!threads_.empty())
    {
        return false;
    }

    threadNum_ = num; // 设置线程数量
    return true;
}

void ZERO_ThreadPool::stop()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);  //加锁

        bTerminate_ = true;     // 触发退出

        condition_.notify_all();
    }

    for (size_t i = 0; i < threads_.size(); i++)
    {
        if(threads_[i]->joinable())
        {
            threads_[i]->join(); // 等线程推出
        }
        delete threads_[i];
        threads_[i] = NULL;
    }

    std::unique_lock<std::mutex> lock(mutex_);
    threads_.clear();
}

bool ZERO_ThreadPool::start()
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (!threads_.empty())
    {
        return false;
    }

    for (size_t i = 0; i < threadNum_; i++)
    {
        threads_.push_back(new thread(&ZERO_ThreadPool::run, this));
    }
    return true;
}

bool ZERO_ThreadPool::get(TaskFuncPtr& task)
{
    std::unique_lock<std::mutex> lock(mutex_); // 也要等锁

    if (tasks_.empty()) // 判断是否任务存在
    {
        condition_.wait(lock, [this] { return bTerminate_  // 要终止线程池 bTerminate_设置为true，外部notify后
                    || !tasks_.empty();  // 任务队列不为空
        }); // notify ->  1. 退出线程池； 2.任务队列不为空
    }

    if (bTerminate_)
        return false;

    if (!tasks_.empty())
    {
        task = std::move(tasks_.front());  // 使用了移动语义

        tasks_.pop(); // 释放一个任务

        return true;
    }

    return false;
}

void ZERO_ThreadPool::run()  // 执行任务的线程
{
    //调用处理部分
    while (!isTerminate()) // 判断是不是要停止
    {
        TaskFuncPtr task;
        bool ok = get(task);        // 1. 读取任务
        if (ok)
        {
            ++atomic_;
            try
            {
                if (task->_expireTime != 0 && task->_expireTime  < TNOWMS )
                {
                    //超时任务，是否需要处理?
                }
                else
                {
                    task->_func();  // 2. 执行任务
                }
            }
            catch (...)
            {
            }

            --atomic_;

            //任务都执行完毕了
            std::unique_lock<std::mutex> lock(mutex_);
            if (atomic_ == 0 && tasks_.empty()) // 3.检测是否所有任务都运行完毕
            {
                condition_.notify_all();  // 这里只是为了通知waitForAllDone
            }
        }
    }
}
// 1000ms
bool ZERO_ThreadPool::waitForAllDone(int millsecond)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (tasks_.empty())
        return true;

    if (millsecond < 0)
    {
        condition_.wait(lock, [this] { return tasks_.empty(); });
        return true;
    }
    else
    {
        return condition_.wait_for(lock, std::chrono::milliseconds(millsecond), [this] { return tasks_.empty(); });
    }
}


int gettimeofday(struct timeval &tv)
{
#if WIN32
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    tm.tm_year   = wtm.wYear - 1900;
    tm.tm_mon   = wtm.wMonth - 1;
    tm.tm_mday   = wtm.wDay;
    tm.tm_hour   = wtm.wHour;
    tm.tm_min   = wtm.wMinute;
    tm.tm_sec   = wtm.wSecond;
    tm. tm_isdst  = -1;
    clock = mktime(&tm);
    tv.tv_sec = clock;
    tv.tv_usec = wtm.wMilliseconds * 1000;

    return 0;
#else
    return ::gettimeofday(&tv, 0);
#endif
}

void getNow(timeval *tv)
{
#if TARGET_PLATFORM_IOS || TARGET_PLATFORM_LINUX

    int idx = _buf_idx;
    *tv = _t[idx];
    if(fabs(_cpu_cycle - 0) < 0.0001 && _use_tsc)
    {
        addTimeOffset(*tv, idx);
    }
    else
    {
        TC_Common::gettimeofday(*tv);
    }
#else
    gettimeofday(*tv);
#endif
}

int64_t getNowMs()
{
    struct timeval tv;
    getNow(&tv);

    return tv.tv_sec * (int64_t)1000 + tv.tv_usec / 1000;
}
