// 2-3-2-forward1
#include <iostream>

using namespace std;

template<class T>
void Print(T &t) {
    cout << "L" << t << endl;
}

template<class T>
void Print(T &&t) {
    cout << "R" << t << endl;
}

template<class T>
void func(T &&t) {
    Print(t);
    Print(std::move(t));
    Print(std::forward<T>(t));
}

int main() {
    cout << "-- func(1)" << endl;
    func(1);
    int x = 10;
    int y = 20;
    cout << "\n-- func(x)" << endl;
    func(x);  // x本身是左值
    cout << "\n-- func(std::forward<int>(y))" << endl;
    func(std::forward<int>(y)); ////T为int，以右值方式转发y
    cout << "\n-- func(std::forward<int&>(y))" << endl;
    func(std::forward<int &>(y));
    cout << "\n-- func(std::forward<int&&>(y))" << endl;
    func(std::forward<int &&>(y));
    return 0;
}
