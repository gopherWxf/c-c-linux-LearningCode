#include "ZeroThreadpool.h"

ZERO_ThreadPool::ZERO_ThreadPool()
        : _threadNum(1), _bTerminate(false) {
}

ZERO_ThreadPool::~ZERO_ThreadPool() {
    stop();
}

bool ZERO_ThreadPool::init(size_t num) {
    std::unique_lock<std::mutex> lock(_mutex);

    if (!_threads.empty()) {
        return false;
    }

    _threadNum = num;
    return true;
}

// 只等当前在执行的任务执行完毕
void ZERO_ThreadPool::stop() {
    {
        std::unique_lock<std::mutex> lock(_mutex);

        _bTerminate = true;

        _condition.notify_all();
    }

    for (size_t i = 0; i < _threads.size(); i++) {
        if (_threads[i]->joinable()) {
            _threads[i]->join();
        }
        delete _threads[i];
        _threads[i] = NULL;
    }

    std::unique_lock<std::mutex> lock(_mutex);
    _threads.clear();
}

bool ZERO_ThreadPool::start() {
    std::unique_lock<std::mutex> lock(_mutex);

    if (!_threads.empty()) {
        return false;
    }

    for (size_t i = 0; i < _threadNum; i++) {
        _threads.push_back(new thread(&ZERO_ThreadPool::run, this));
    }
    return true;
}

bool ZERO_ThreadPool::get(TaskFuncPtr &task) {
    std::unique_lock<std::mutex> lock(_mutex);

    if (_tasks.empty()) {
        _condition.wait(lock, [this] { return _bTerminate || !_tasks.empty(); });
    }

    if (_bTerminate)
        return false;

    if (!_tasks.empty()) {
        task = std::move(_tasks.front());  // 使用了移动语义，取任务

        _tasks.pop();  // 任务出队列

        return true;
    }

    return false;
}

void ZERO_ThreadPool::run()  // 执行任务的线程
{
    int64_t start_time = getNowMs();
    //调用处理部分
    while (!isTerminate()) // 判断是不是要停止
    {
        TaskFuncPtr task;
        bool ok = get(task);        // 读取任务
        if (ok) {
            ++_atomic;  // 原子变量有什么用呢？
            try {
                if (task->_expireTime != 0 && task->_expireTime < TNOWMS) {
                    //超时任务，是否需要处理?
                }
                else {
                    task->_func();  // 执行任务
                }
            }
            catch (...) {
            }

            --_atomic;

            //任务都执行完毕了
            std::unique_lock<std::mutex> lock(_mutex);
            if (_atomic == 0 && _tasks.empty()) {
                cout << "need time: " << getNowMs() - start_time << "ms\n";
                _condition.notify_all();  // 这里只是为了通知waitForAllDone
            }

//            if (_tasks.empty())  {
//                _condition.notify_all();  // 这里只是为了通知waitForAllDone
//            }
        }
    }
}

// 等待所有的任务执行完毕，包括任务队列的，但是这里是可以带超时的，如果超时了也还没有执行完毕，那我就不等了。
bool ZERO_ThreadPool::waitForAllDone(int millsecond) {
    std::unique_lock<std::mutex> lock(_mutex);

    if (_tasks.empty())
        return true;

    if (millsecond < 0) {
        _condition.wait(lock, [this] { return _tasks.empty(); });
        return true;
    }
    else {
        return _condition.wait_for(lock, std::chrono::milliseconds(millsecond), [this] { return _tasks.empty(); });
    }
}


int gettimeofday(struct timeval &tv) {
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

void getNow(timeval *tv) {
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

int64_t getNowMs() {
    struct timeval tv;
    getNow(&tv);

    return tv.tv_sec * (int64_t) 1000 + tv.tv_usec / 1000;
}
