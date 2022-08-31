//1-5-package_task
#include <iostream>
#include <future>
#include <thread>

using namespace std;

int add(int a, int b, int c)
{
    std::cout << "call add\n";
    return a + b + c;
}

void do_other_things()
{
    std::cout << "do_other_things" << std::endl;
}

int main()
{
    std::packaged_task<int(int, int, int)> task(add);  // 1. 封装任务，还没有运行
    std::this_thread::sleep_for(std::chrono::seconds(5)); // 用来测试异步延迟的影响

//    do_other_things();
    std::future<int> result = task.get_future(); // 这里运行吗？这里只是获取 future
    // 这里才真正运行
    task(1, 1, 2);   //必须要让任务执行，否则在get()获取future的值时会一直阻塞
    std::cout << "result:" << result.get() << std::endl;
    return 0;
}
