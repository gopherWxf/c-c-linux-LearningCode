#include <iostream>
#include <functional> // 添加头文件 functional
using namespace std;

// function 类似c的函数指针
//保存普通函数
void func1(int a)
{
     cout << a << endl;
}
//保存成员函数
class A{
public:
    A(string name) : name_(name){}
    void func3(int i) const {cout <<name_ << ", " << i << endl;}
    // void func3(string str) const {cout <<name_ << ", " << str << endl;}

    //    std::bind((void(A::*)(int, int))&A::fun_4
    void func4(int k,int m)
    {
        cout<<"func4 print: k="<<k<<",m="<<m<<endl;
    }
//    std::bind((void(A::*)(string))&A::fun_4
    void func4(string str) {
        cout<<"func4 print: str="<<str<<endl;
    }
private:
    string name_;
};
int main()
{
    cout << "main1 -----------------" << endl;
    //1. 保存普通函数
    std::function<void(int a)> func1_;
    func1_ = func1;
    func1_(2);   //2

     cout << "\n\nmain2 -----------------" << endl;
    //2. 保存lambda表达式
    std::function<void()> func2_ = [](){cout << "hello lambda" << endl;};
    func2_();  //hello world

    cout << "\n\nmain3 -----------------" << endl;
    //3 保存成员函数
    std::function<void(const A&, int)> func3_ = &A::func3;
    A a("darren");
    func3_(a, 1);

    //4.重载函数
    std::function<void(int, int)> func4_1 = std::bind((void(A::*)(int, int))&A::func4, a,std::placeholders::_1, std::placeholders::_2); 
    func4_1(1,2);
    auto f_str = std::bind((void(A::*)(string))&A::func4, a,std::placeholders::_1);
    f_str("darren");
    std::function<void(string)> f_str2 = std::bind((void(A::*)(string))&A::func4, &a,std::placeholders::_1);

    return 0;
}
