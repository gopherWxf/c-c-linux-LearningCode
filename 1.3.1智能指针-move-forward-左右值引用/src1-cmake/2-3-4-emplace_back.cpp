//2-5-emplace_back
#include <vector>
#include <string>
#include "time_interval.h"

int main() {


    std::vector<std::string> v;
    int count = 10000000;
    v.reserve(count);       //预分配十万大小，排除掉分配内存的时间
    {
        TIME_INTERVAL_SCOPE("push_back string:");
        for (int i = 0; i < count; i++) {
            std::string temp("ceshi");
            v.push_back(temp);// push_back(const string&)，参数是左值引用
        }
    }

    v.clear();
    {
        TIME_INTERVAL_SCOPE("push_back move(string):");
        for (int i = 0; i < count; i++) {
            std::string temp("ceshi");
            v.push_back(std::move(temp));// push_back(string &&), 参数是右值引用
        }
    }

    v.clear();
    {
        TIME_INTERVAL_SCOPE("push_back(string):");
        for (int i = 0; i < count; i++) {
            v.push_back(std::string("ceshi"));// push_back(string &&), 参数是右值引用
        }
    }

    v.clear();
    {
        TIME_INTERVAL_SCOPE("push_back(c string):");
        for (int i = 0; i < count; i++) {
            v.push_back("ceshi");// push_back(string &&), 参数是右值引用
        }
    }

    v.clear();
    {
        TIME_INTERVAL_SCOPE("emplace_back(c string):");
        for (int i = 0; i < count; i++) {
            v.emplace_back("ceshi");// 只有一次构造函数，不调用拷贝构造函数，速度最快
        }
    }
}

