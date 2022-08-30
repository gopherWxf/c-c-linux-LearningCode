#include <iostream>

using namespace std;

void test1() {
    cout << "test2!" << endl;
    int a = 5; // a是个左值
    int &ref_a_left = a; // 左值引用指向左值
    int &&ref_a_right = std::move(a); // 通过std::move将左值转化为右值，可以被右值引用指向

    cout << "a:" << a << endl; // 打印结果：5
}

void test2() {
    cout << "test2!" << endl;
    int &&ref_a = 5;
    ref_a = 6;
}

void test3() {
    cout << "test3!" << endl;
    int temp = 5;
    int &&ref_a = std::move(temp);  //
    ref_a = 6;
    cout << "temp:" << temp << ", ref_a:" << ref_a << endl;
}

int main() {

    test1();
    test2();
    test3();
    return 0;
}

