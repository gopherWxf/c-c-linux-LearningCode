//2-1-memory2
#include <iostream>

using namespace std;

class A {
public:
    A() : m_ptr(new int(0)) {
        cout << "constructor A" << endl;
    }

    A(const A &a) : m_ptr(new int(*a.m_ptr)) {
        cout << "copy constructor A" << endl;
    }

    ~A() {
        cout << "destructor A, m_ptr:" << m_ptr << endl;
        delete m_ptr;
        m_ptr = nullptr;
    }

private:
    int *m_ptr;
};

// 为了避免返回值优化，此函数故意这样写
A Get(bool flag) {
    A a;
    A b;
    cout << "ready return" << endl;
    if (flag)
        return a;
    else
        return b;
}

int main() {
    {
        A a = Get(false); // 正确运行
    }
    cout << "main finish" << endl;
    return 0;
}
