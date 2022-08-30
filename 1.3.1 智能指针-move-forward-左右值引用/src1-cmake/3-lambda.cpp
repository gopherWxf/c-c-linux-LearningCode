#include <iostream>
#include <memory>

using namespace std;

//匿名函数的基本语法为
//[捕获列表](参数列表)->返回类型{函数体}
void test1() {
    cout << "test1" << endl;
    auto Add = [](int a, int b) -> int {
        return a + b;
    };
    std::cout << Add(1, 2) << std::endl;        //输出3
}

//一般情况下，编译器可以自动推断出lambda表达式的返回类型，所以我们可以不指定返回类型
//[捕获列表](参数列表){函数体}
void test2() {
    cout << "test2" << endl;
    auto Add = [](int a, int b) {
        return a + b;
    };
    std::cout << Add(1, 2) << std::endl;        //输出3
}

// 按值传递
void test3() {
    cout << "test3" << endl;
    int c = 12;
    int d = 30;
    int e = 40;
    auto Add = [c, d, &e](int a, int b) -> int {
        cout << "d = " << d << endl;
        cout << "e = " << e << endl;
        return c;
    };
    d = 20;
    e = 20;
    std::cout << Add(1, 2) << std::endl;
}

void test4() {
    cout << "test4" << endl;
    int c = 12;
    auto Add = [c](int a, int b) -> int {
//        c = a; // 编译报错
        return c;
    };
    std::cout << Add(1, 2) << std::endl;
}

// 引用传递
void test5() {
    cout << "test5" << endl;
    int c = 12;
    int d = 30;
    auto Add = [&c, &d](int a, int b) -> int {
        c = a; // 编译对的
        cout << "d = " << d << endl;
        return c;
    };
    d = 20;
    std::cout << Add(1, 2) << std::endl;
}

// 引用传递
void test6() {
    cout << "test6" << endl;
    int c = 12;
    int d = 30;
    auto Add = [&c, &d](int &a, int &b) -> int {
        a = 11;
        b = 12;
        cout << "d = " << d << endl;
        return a + b;
    };
    d = 20;
    std::cout << Add(c, d) << std::endl;
    cout << "c = " << d << endl;
    cout << "d = " << d << endl;
}

//  隐式捕获
//手动书写捕获列表有时候是非常复杂的，这种机械性的工作可以交给编译器来处理，这时候可以在捕获列表中写一个 & 或 = 向编译器声明采用引用捕获或者值捕获。
void test7() {
    cout << "test7" << endl;
    int c = 12;
    int d = 30;

    // 把捕获列表的&改成=再测试
    auto Add = [&](int a, int b) -> int {
        c = a; // 编译对的
        cout << "d = " << d << endl;
        return c;
    };
    d = 20;
    std::cout << Add(1, 2) << std::endl;
    std::cout << "c:" << c << std::endl;
}

void test8() {
    cout << "test7" << endl;
    int c = 12;
    int d = 30;

    // 把捕获列表的&改成=再测试
    // [] 空值，不能使用外面的变量
    // [=] 传值，lambda外部的变量都能使用
    // [&] 传引用值，lambda外部的变量都能使用
    auto Add = [&](int a, int b) -> int {
        cout << "d = " << d << endl; // 编译报错
        return c;// 编译报错
    };
    d = 20;
    std::cout << Add(1, 2) << std::endl;
    std::cout << "c:" << c << std::endl;
}

//5. 表达式捕获 C++14之后支持捕获右值，允许捕获的成员用任意的表达式进行初始化
void test9() {
    cout << "test9" << endl;
    auto important = std::make_unique<int>(1);

    auto add = [v1 = 1, v2 = std::move(important)](int x, int y) -> int {
        return x + y + v1 + (*v2);
    };

    std::cout << add(3, 4) << std::endl;
}

//泛型 Lambda C++14
void test10() {
    cout << "test10" << endl;
    auto add = [](auto x, auto y) {
        return x + y;
    };

    std::cout << add(1, 2) << std::endl;
    std::cout << add(1.1, 1.2) << std::endl;
}
//混合使用隐式捕获和显式捕获

//要求捕获列表中第一个元素必须是隐式捕获（&或=）
//混合使用时，若隐式捕获采用 引用捕获 &，则显式捕获的变量必须采用值捕获的方式
//若隐式捕获采用值捕获 = ，则显式捕获的变量必须采用引用捕获的方式，即变量名前加&
void test11() {
    cout << "test1" << endl;
    int c = 12;
    int d = 30;
    int e = 30;
    // &, d, e 代表除了d其他都是引用传递
    // =, &c 代表除了d其他都是引用传递
//    auto Add = [&, d, e](int a, int b)->int {
    auto Add = [=, &c](int a, int b) -> int {
        c = a; // 编译对的
        cout << "d = " << d << ", e:" << e << endl;
        return c;
    };
    d = 20;
    std::cout << Add(1, 2) << std::endl;
    std::cout << "c:" << c << std::endl;
}

void test12() {
    cout << "test12" << endl;
    int v = 5;
    // 值捕获方式，使用mutable修饰，可以改变捕获的变量值
    auto ff = [v]() mutable { return ++v; };

    v = 0;
    auto j = ff();  // j为6
    cout << "j:" << j << endl;
}

void test13() {
    cout << "test13" << endl;
    int v = 5;
    // 采用引用捕获方式，可以直接修改变量值
    auto ff = [&v] { return ++v; };

    v = 0;
    auto j = ff();  // v引用已修改，j为1
    cout << "j:" << j << endl;
}

int main() {
    test8();
//    test2();
//    test3();
//    test4();
//    test5();
//    test6();
//    test12();
//    test13();
    return 0;
}
