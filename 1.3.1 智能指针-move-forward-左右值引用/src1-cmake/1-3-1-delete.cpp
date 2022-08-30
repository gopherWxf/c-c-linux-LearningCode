#include <iostream>
#include <memory>

using namespace std;

void DeleteIntPtr(int *p) {
    cout << "call DeleteIntPtr" << endl;
    delete p;
}

int main() {

    int *ptr = new int;
    shared_ptr<int> p1(ptr);
    shared_ptr<int> p2(ptr); // 逻辑错误

    std::shared_ptr<int> p(new int(1), DeleteIntPtr);
    std::shared_ptr<int> p2(new int(1), [](int *p) {
        cout << "call lambda1 delete p" << endl;
        delete p;
    });
    std::shared_ptr<int> p3(new int[10], [](int *p) {
        cout << "call lambda2 delete p" << endl;
        delete[] p; // 数组删除
    });
    return 0;
}
