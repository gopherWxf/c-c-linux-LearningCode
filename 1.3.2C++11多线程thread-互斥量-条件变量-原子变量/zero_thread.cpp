#include "zero_thread.h"
#include <sstream>
#include <iostream>
#include <exception>

ZERO_Thread::ZERO_Thread() :
        running_(false), th_(NULL) {

}

ZERO_Thread::~ZERO_Thread() {
    if (th_ != NULL) {
        //如果到调用析构函数的时候，调用者还没有调用join则触发detach，此时是一个比较危险的动作，用户必须知道他在做什么
        if (th_->joinable()) {
            std::cout << "~ZERO_Thread detach\n";
            th_->detach();
        }

        delete th_;
        th_ = NULL;
    }
    std::cout << "~ZERO_Thread()" << std::endl;
}

bool ZERO_Thread::start() {
    if (running_) {
        return false;
    }
    try {
        th_ = new std::thread(&ZERO_Thread::threadEntry, this);
    }
    catch (...) {
        throw "[ZERO_Thread::start] thread start error";
    }
    return true;
}

void ZERO_Thread::stop() {
    running_ = false;
}

bool ZERO_Thread::isAlive() const {
    return running_;
}

void ZERO_Thread::join() {
    if (th_->joinable()) {
        th_->join();  // 不是detach才去join
    }
}

void ZERO_Thread::detach() {
    th_->detach();
}

size_t ZERO_Thread::CURRENT_THREADID() {
    // 声明为thread_local的本地变量在线程中是持续存在的，不同于普通临时变量的生命周期，
    // 它具有static变量一样的初始化特征和生命周期，即使它不被声明为static。
    static thread_local size_t
    threadId = 0;
    if (threadId == 0) {
        std::stringstream ss;
        ss << std::this_thread::get_id();
        threadId = strtol(ss.str().c_str(), NULL, 0);
    }
    return threadId;
}

void ZERO_Thread::threadEntry() {
    running_ = true;

    try {
        run();   // 函数运行所在 调用子类的run函数
    }
    catch (std::exception &ex) {
        running_ = false;
        throw ex;
    }
    catch (...) {
        running_ = false;
        throw;
    }
    running_ = false;
}
