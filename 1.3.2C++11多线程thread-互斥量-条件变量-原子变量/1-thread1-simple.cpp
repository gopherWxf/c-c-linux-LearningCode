#include <iostream>
#include <thread> // 头文件
using namespace std;

// 1 传入0个值
void func1()
{
    cout << "func1 into" << endl;
}


// 2 传入2个值
void func2(int a, int b)
{
    cout << "func2 a + b = " << a+b << endl;
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

// 3 传入引用
void func3(int &c) // 引用传递
{
    cout << "func3 c = " << &c << endl;
    c += 10;
}



//
class A
{
public:
//    4. 传入类函数
    void func4(int a)
    {
//        std::this_thread::sleep_for(std::chrono::seconds(1));
        cout << "thread:" << name_<< ", fun4 a = " << a << endl;

    }
    int func4(string str)
    {
//        std::this_thread::sleep_for(std::chrono::seconds(1));
        cout << "thread:" << name_<< ", fun4 str = " << str << endl;

    }
    void setName(string name) {
        name_ = name;
    }
    void displayName() {
        cout << "this:"  << this << ", name:" << name_ << endl;
    }
    void play()
    {
        std::cout<<"play call!"<<std::endl;
    }
private:
    string name_;
};

//5. detach
void func5()
{
    cout << "func5 into sleep " <<  endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    cout << "func5 leave " <<  endl;
}

// 6. move
void func6()
{
    cout << "this is func6 !" <<endl;
}


int main()
{
////    1. 传入0个值
//    cout << "\n\n main1--------------------------\n";
//    std::thread t1(func1);  // 只传递函数
//    t1.join();  // 阻塞等待线程函数执行结束




////    2. 传入2个值
cout << "\n\n main2--------------------------\n";
//    int a =10;
//    int b =20;
//    std::thread t2(func2, a, b); // 加上参数传递,可以任意参数
//    t2.join();

    int a =10;
    int b =20;
    std::thread t2((void(*)(int, int)) func2_1, a, b); // 加上参数传递,可以任意参数
    t2.join();

    std::thread t2_2((int(*)(string, string)) func2_1, "darren", " and mark"); // 加上参数传递,可以任意参数
    t2_2.join();


////    3. 传入引用
//    cout << "\n\n main3--------------------------\n";
//    int c =10;
//    std::thread t3(func3, std::ref(c)); // std::ref 加上参数传递,可以任意参数
//    t3.join();
//    cout << "main3 c = " << &c << ", "<<c << endl;

    //  4. 传入类函数
    // 普通
    cout << "\n\n main4--------------------------\n";
//    A * a4_ptr = new A();
//    a4_ptr->setName("darren");
//    std::thread t4(A::func4, a4_ptr, 10);
//    t4.join();
//    delete a4_ptr;

    // 重载
    A * a4_ptr2 = new A();
    a4_ptr2->setName("king");
    std::thread t41((void(A::*)(int))&A::func4, a4_ptr2, 100);      // 重载void func4(int a)
    t41.join();
    delete a4_ptr2;

    // 重载
    A * a4_ptr3 = new A();
    a4_ptr3->setName("Darren");
    std::thread t43((int(A::*)(string))&A::func4, a4_ptr3, "and mark"); // 重载 int func4(string str)
    t43.join();
    delete a4_ptr3;

//    //   5.detach
//    cout << "\n\n main5--------------------------\n";
//    std::thread t5(&func5);  // 只传递函数
//    t5.detach();  // 脱离
//     cout << "pid: " << t5.get_id() << endl; // t5此时不能管理线程了
//    cout << "joinable: " << t5.joinable() << endl; // false
////    t5.join();
////    std::this_thread::sleep_for(std::chrono::seconds(2)); // 如果这里不休眠会怎么样
//    cout << "\n main5 end\n";
//    // 6.move
    cout << "\n\n main6--------------------------\n";
    int x = 10;
   thread t6_1(func6);
   thread t6_2(std::move(t6_1)); // t6_1 线程失去所有权
    t6_1.join();  // 抛出异常   after throwing an instance of 'std::system_error'
    t6_2.join();

    return 0;
}
