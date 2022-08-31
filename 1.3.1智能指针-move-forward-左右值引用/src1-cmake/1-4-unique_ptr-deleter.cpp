#include <iostream>
#include <memory>

using namespace std;

struct MyDelete {
    void operator()(int *p) {
        cout << "delete" << endl;
        delete p;
    }
};

int main() {
    auto upw1(std::make_unique<int[]>(2));     // with make func
    std::unique_ptr<int> upw2(new int); // without make func
    auto upw3 = std::make_unique<int[]>(2);     // with make func

    upw1[21] = 1;
    std::unique_ptr<int, MyDelete> ptr2(new int(1));
//    auto ptr(std::make_unique<int, MyDelete>(1));
    cout << "main finish! " << upw1[1] << endl;
    return 0;
}
