#include <iostream>
#include <thread>
#include <memory>
#include <mutex>
#include <atomic>
#include <queue>
#include <string>
#include <condition_variable>
#include <functional>
#include <string.h>

using namespace std;

class Buffer {
public:
    Buffer(const char *str) {
        size_t size = strlen(str);
        ptr_ = new char[size + 1];
        memcpy(ptr_, str, size);
        ptr_[size] = '\0';
        cout << "Buffer Constructor, ptr:" << ptr_ << endl;
    }

    const char *get() {
        return ptr_;
    }

    ~Buffer() {
        cout << "Buffer Destructor, ptr:" << ptr_ << endl;
        if (ptr_) {
            delete[] ptr_;
        }
    }


private:
    char *ptr_ = nullptr;
    size_t size = 0;
};

// 封装一个线程
class Thread {
public:
    Thread(string name) : name_(name) {
        cout << "Thread Constructor" << endl;
    }

    virtual ~Thread() {
        cout << "Thread Destructor" << endl;
        if (!IsTerminate()) {
            this->Stop();
        }

    }

    void Start() {
        std::thread thr(std::bind(&Thread::Run, this));
        thread_ = std::move(thr);  // mov语义
    }

    std::thread::id GetId() {
        return thread_.get_id();
    }

    void Stop() {
        { // 注意这里加 {}的作用， 是为了能释放mutex_的占用
            unique_lock<mutex> lock(mutex_);
            terminate_ = true;
            condition_.notify_one();
        }
//        cout << "Stop terminate_: " << terminate_ << endl;

        if (thread_.joinable()) {
//            cout << "wait thread exit\n";
            thread_.join();
        }
    }

    bool IsTerminate() {
        return terminate_;
    }

    virtual void Run() = 0;

protected:
    string name_;
    bool terminate_ = false;
    std::thread thread_;
    mutex mutex_;
    condition_variable condition_;
};

class MyThread :
        public Thread {
public:
    MyThread(string name) : Thread(name) {
        cout << "Thread name: " << name_;
    }

    virtual ~MyThread() {

    }

    void Push(shared_ptr<Buffer> buf) {
        unique_lock<mutex> lock(mutex_);
        buf_queue_.push(buf);
        condition_.notify_one();
    }

    void Stop2() {
        unique_lock<mutex> lock(mutex_);
        terminate_ = true;
        condition_.notify_one();
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    virtual void Run() override {
        while (!IsTerminate()) {
            // 处理具体的业务
            shared_ptr<Buffer> buf;
            bool ok = get(buf);
            if (ok) {
                cout << name_ << " handle " << buf->get() << endl; // 处理具体事情，这里只是模拟不同的业务处理同样的数据
            }
        }
    }

    bool get(shared_ptr<Buffer> &buf) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (buf_queue_.empty()) {
//            cout <<" wait into" << endl;
            // 它会去检查谓词对象的bool返回值是否是true, 如果是true才真正唤醒，否则继续block
            condition_.wait(lock, [this] {
//                cout <<" wait check terminate_:" << terminate_ << ", queue:" << !buf_queue_.empty() << endl;
                return terminate_ || !buf_queue_.empty();
            });
//            cout <<" wait leave" << endl;
        }

        if (terminate_)
            return false;

        if (!buf_queue_.empty()) {
            buf = std::move(buf_queue_.front());  // 使用了移动语义
            buf_queue_.pop();
            return true;
        }

        return false;
    }

private:
    queue<shared_ptr<Buffer>> buf_queue_;
};


int main1() {

    {
        shared_ptr<Buffer> buf = make_shared<Buffer>("auto free memory");
    }
    cout << "\n\n---------------\n\n";
    {
        MyThread thread_a("Thread A");
        MyThread thread_b("Thread B");

        shared_ptr<Buffer> buf1 = make_shared<Buffer>("01234");
        shared_ptr<Buffer> buf2 = make_shared<Buffer>("56789");
        shared_ptr<Buffer> buf3 = make_shared<Buffer>("abcde");

        thread_a.Start();
        thread_b.Start();
        thread_a.Push(buf1);
        thread_b.Push(buf1);
        thread_a.Push(buf2);
        thread_b.Push(buf2);
        thread_a.Push(buf3);
        thread_b.Push(buf3);
        this_thread::sleep_for(std::chrono::seconds(1));
        cout << "sleep_for end" << endl;
//        thread_a.Push(buf1);
//        thread_b.Push(buf1);
//        this_thread::sleep_for(std::chrono::seconds(1));
        thread_a.Stop();
        thread_b.Stop();
    }
    cout << "main end" << endl;
    return 0;
}
