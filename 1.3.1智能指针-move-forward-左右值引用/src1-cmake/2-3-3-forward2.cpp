//2-3-3-forward2
#include "stdio.h"
#include <iostream>
#include <cstring>
#include <vector>

using namespace std;

class A {
public:
    A() : m_ptr(NULL), m_nSize(0) {}

    A(int *ptr, int nSize) {
        m_nSize = nSize;
        m_ptr = new int[nSize];
        printf("A(int *ptr, int nSize) m_ptr:%p\n", m_ptr);
        if (m_ptr) {
            memcpy(m_ptr, ptr, sizeof(sizeof(int) * nSize));
        }
    }

    A(const A &other) // 拷贝构造函数实现深拷贝
    {
        m_nSize = other.m_nSize;
        if (other.m_ptr) {
            printf("A(const A &other) m_ptr:%p\n", m_ptr);
            if (m_ptr)
                delete[] m_ptr;
            printf("delete[] m_ptr\n");
            m_ptr = new int[m_nSize];
            memcpy(m_ptr, other.m_ptr, sizeof(sizeof(int) * m_nSize));
        }
        else {
            if (m_ptr)
                delete[] m_ptr;
            m_ptr = NULL;
        }
        cout << "A(const int &i)" << endl;
    }

    // 右值引用移动构造函数
    A(A &&other) {
        m_ptr = NULL;
        m_nSize = other.m_nSize;
        if (other.m_ptr) {
            m_ptr = move(other.m_ptr); // 移动语义
            other.m_ptr = NULL;
        }
    }

    ~A() {
        if (m_ptr) {
            delete[] m_ptr;
            m_ptr = NULL;
        }
    }

    void deleteptr() {
        if (m_ptr) {
            delete[] m_ptr;
            m_ptr = NULL;
        }
    }

    int *m_ptr = NULL;  // 增加初始化
    int m_nSize = 0;
};

int main() {
    int arr[] = {1, 2, 3};
    A a(arr, sizeof(arr) / sizeof(arr[0]));
    cout << "m_ptr in a Addr: 0x" << a.m_ptr << endl;
    A b(a);
    cout << "m_ptr in b Addr: 0x" << b.m_ptr << endl;

    b.deleteptr();
    A c(std::forward<A>(a)); // 完美转换
    cout << "m_ptr in c Addr: 0x" << c.m_ptr << endl;
    c.deleteptr();
    vector<int> vect{1, 2, 3, 4, 5};
    cout << "before move vect size: " << vect.size() << endl;
    vector<int> vect1 = move(vect);
    cout << "after move vect size: " << vect.size() << endl;
    cout << "new vect1 size: " << vect1.size() << endl;
    return 0;
}
