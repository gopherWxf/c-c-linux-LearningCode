//1-5-promise
// std::promise和std::future配合，可以在线程之间传递数据。
#include <future>
#include <string>
#include <thread>
#include <iostream>
using namespace std;
void print1(std::promise<std::string>& p)
{
    std::cout << "print1 sleep" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    p.set_value("set string"); // 返回future的结果
}

void print2(std::promise<int>& p)
{
    std::cout << "print2 sleep" << std::endl;
    p.set_value(1);
}

void do_some_other_things()
{
    std::cout << "do_some_other_things" << std::endl;
}

int main()
{
    std::cout << "main1 -------------" << std::endl;
    std::promise<std::string> promise;  // 注意类型:

    std::future<std::string> result = promise.get_future(); // future

    std::thread t(print1, std::ref(promise));  // 线程设置 传引用std::ref
    do_some_other_things();
    std::cout << "wait get result" << std::endl;
    std::cout <<"result " << result.get() << std::endl; // 在主线程等待 promise的返回 result set string
    t.join();

//    std::cout << "\n\nmain2 -------------" << std::endl;
//    std::promise<int> promise2;

    // std::future<int> result2 = promise2.get_future();
//    std::thread t2(print2, std::ref(promise2));
//    do_some_other_things();
//    std::cout << "result2 " << result2.get() << std::endl;
//    t2.join();
    return 0;
}
