#include <iostream>
#include <functional>
using namespace std;
class A
{
public:
    // 重载fun_3，主要bind的时候需要
//    std::bind((void(A::*)(int, int))&A::fun_3
    void fun_3(int k,int m)
    {
        cout << "fun_3 a = " << a<< endl;
        cout<<"print: k="<<k<<",m="<<m<<endl;
    }
//    std::bind((void(A::*)(string))&A::fun_3
    void fun_3(string str) {
        cout<<"print: str="<<str<<endl;
    }
    int a;
};

void fun_1(int x,int y,int z)
{
    cout<<"fun_1 print: x=" <<x<<",y="<< y << ",z=" <<z<<endl;
}

void fun_2(int &a,int &b)
{
    a++;
    b++;
    cout<<"print: a=" <<a<<",b="<<b<<endl;
}

void func2_1(int a, int b)
{
    cout << "func2_1 a + b = " << a+b << endl;
}

int func2_1(string a, string b)
{
    cout << "func2_1 a + b = " << a << b<< endl;
    return 0;
}

int main()
{
    auto f2_1_1 = std::bind((void(*)(int, int))func2_1, 1, 2); //表示绑定函数 fun 的第一，二，三个参数值为： 1 2 3
    f2_1_1(); //print: x=1,y=2,z=3



#if 0
    //f1的类型为 function<void(int, int, int)>
    cout << "\n\nstd::bind(fun_1, 1, 2, 3) -----------------\n";
    auto f1 = std::bind(fun_1, 1, 2, 3); //表示绑定函数 fun 的第一，二，三个参数值为： 1 2 3
    f1(); //print: x=1,y=2,z=3

    cout << "\n\nstd::bind(fun_1, 10, 20, 30) -----------------\n";
    auto f11 = std::bind(fun_1, 10, 20, 30); //表示绑定函数 fun 的第一，二，三个参数值为： 1 2 3
    f11();


    cout << "\n\nstd::bind(fun_1, placeholders::_1,placeholders::_2, 3) -----------------\n";
    auto f2 = std::bind(fun_1, placeholders::_1, placeholders::_2, 3);
    //表示绑定函数 fun 的第三个参数为 3，而fun 的第一，二个参数分别由调用 f2 的第一，二个参数指定
    f2(1,2);//print: x=1,y=2,z=3
    f2(10,21,30); // 传入30也没有用
#endif

//    cout << "\n\nstd::bind(fun_1,placeholders::_2,placeholders::_1,3) -----------------\n";
//    auto f3 = std::bind(fun_1,placeholders::_2,placeholders::_1,3);
//    //表示绑定函数 fun 的第三个参数为 3，而fun 的第一，二个参数分别由调用 f3 的第二，一个参数指定
//    //注意： f2  和  f3 的区别。
//    f3(1,2);//print: x=2,y=1,z=3

//    cout << "\n\nstd::bind(fun_2, placeholders::_1, n) -----------------\n";
//    int m = 2;
//    int n = 3;
//    auto f4 = std::bind(fun_2, placeholders::_1, n); //表示绑定fun_2的第一个参数为n, fun_2的第二个参数由调用f4的第一个参数（_1）指定。
//    f4(m); //print: m=3,n=4
//    cout<<"m="<<m<<endl;//m=3  说明：bind对于不事先绑定的参数，通过std::placeholders传递的参数是通过引用传递的,如m
//    cout<<"n="<<n<<endl;//n=3  说明：bind对于预先绑定的函数参数是通过值传递的，如n
//#if 1

    cout << "\n\nstd::bind(&A::fun_3, &a,placeholders::_1,placeholders::_2) -----------------\n";
    A a;
    a.a = 10;
    //f5的类型为 function<void(int, int)>
    auto f5 = std::bind((void(A::*)(int, int))&A::fun_3, &a, 40, 50); //使用auto关键字
    f5(10,20);//调用a.fun_3(10,20),print: k=10,m=20

    cout << "\n\nstd::bind(&A::fun_3, &a2,placeholders::_1,placeholders::_2) -----------------\n";
    A a2;
    a2.a = 20;
    //f5的类型为 function<void(int, int)>
    auto f6 = std::bind((void(A::*)(int, int))&A::fun_3, &a2,placeholders::_1,placeholders::_2); //使用auto关键字
    f6(10,20);//

//    cout << "\n\nstd::bind(&A::fun_3, a,std::placeholders::_1,std::placeholders::_2) -----------------\n";
//    std::function<void(int,int)> fc = std::bind((void(A::*)(int, int))&A::fun_3, a,std::placeholders::_1,std::placeholders::_2);
//    fc(10,20);   //调用a.fun_3(10,20) print: k=10,m=20
//    fc = std::bind((void(A::*)(int, int))&A::fun_3, a2,std::placeholders::_1,std::placeholders::_2);


    cout << "\n\nstd::bind(&A::fun_3, a,std::placeholders::_1,std::placeholders::_2) -----------------\n";
    auto f_str = std::bind((void(A::*)(string))&A::fun_3, a,std::placeholders::_1);
    f_str("darren");
//#endif
    return 0;
}
