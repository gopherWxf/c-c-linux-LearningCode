//2-1-memory
#include <iostream>

using namespace std;

class A {
public:
    A() : m_ptr(new int(0)) {
        cout << "constructor A" << endl;
    }

//    A(const A& a) :m_ptr(new int(*a.m_ptr)) {  // 拷贝构造函数
//       cout << "copy constructor A"  << endl;
//   }
    ~A() {
        cout << "destructor A, m_ptr:" << m_ptr << endl;
        delete m_ptr;
        m_ptr = nullptr;
    }

private:
    int *m_ptr;
};

// 为了避免返回值被优化，此函数故意这样写
A Get(bool flag) {
    A a1;
    A a2;
    cout << "ready return" << endl;
    if (flag)
        return a1;
    else
        return a2;
}

int main() {

    {
        printf("Get before\n");
        A a = Get(false); // 运行报错
        printf("Get after\n");
    }
    cout << "main finish" << endl;
    return 0;
}
