//1-5-future
#include <iostream>
#include <future>
#include <thread>
using namespace std;

int find_result_to_add()
{
    std::this_thread::sleep_for(std::chrono::seconds(5)); // 用来测试异步延迟的影响
    std::cout << "find_result_to_add" << std::endl;
    return 1 + 1;
}

int find_result_to_add2(int a, int b)
{
//    std::this_thread::sleep_for(std::chrono::seconds(5)); // 用来测试异步延迟的影响
    return a + b;
}

void do_other_things()
{
    std::cout << "do_other_things" << std::endl;
//    std::this_thread::sleep_for(std::chrono::seconds(5));
}

int main()
{
//    std::future<T>                     std::async他是异步线程
//    std::future<int> result = std::async(find_result_to_add);  // 不会阻塞32行的运行
//    std::future<decltype (find_result_to_add())> result = std::async(find_result_to_add);
//    auto result = std::async(find_result_to_add);  // 推荐的写法
//    do_other_things();
//    std::cout << "result: " << result.get() << std::endl;  // 延迟是否有影响？

//    std::future<decltype(find_result_to_add2(int, int))> result2 = std::async(find_result_to_add2, 10, 20); //错误
    std::future<decltype (find_result_to_add2(0, 0))> result2 = std::async(find_result_to_add2, 10, 20);
    std::cout << "result2: " << result2.get() << std::endl;  // 延迟是否有影响？
////    std::cout << "main finish" << endl;
    return 0;
}
