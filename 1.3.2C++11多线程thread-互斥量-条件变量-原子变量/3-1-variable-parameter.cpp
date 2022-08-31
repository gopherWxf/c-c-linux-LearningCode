//3-1-variable-parameter 一个简单的可变模版参数函数
#include <iostream>

using namespace std;

template <class... T>
void f(T... args)
{
    cout << sizeof...(args) << endl; //打印变参的个数
}
int main()
{    
    f();        //0
    f(1, 2);    //2
    f(1, 2.5, "");    //3
    return 0;
}
