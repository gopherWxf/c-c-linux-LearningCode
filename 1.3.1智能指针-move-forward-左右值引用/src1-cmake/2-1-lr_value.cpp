#include <iostream>

using namespace std;

/*
1. 什么是左值、右值
- 左值可以取地址、位于等号左边；
- 而右值没法取地址，位于等号右边。
*/

void test1() {
    struct A {
        A(int a = 0) {
            a_ = a;
        }

        int a_;
    };

    A a = A();
}

int main() {
    cout << "Hello World!" << endl;
    return 0;
}
