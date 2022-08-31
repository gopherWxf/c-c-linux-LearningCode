#include <iostream>
#include <memory>

using namespace std;

void test1() {
    unique_ptr<int> my_ptr(new int); // 正确
    if (!my_ptr) {
        cout << "1 my_ptr is null" << endl;
    }

    unique_ptr<int> my_other_ptr = std::move(my_ptr); // 正确
    if (!my_ptr) {
        cout << "2 my_ptr is null" << endl;
    }
    if (!my_other_ptr) {
        cout << "2 my_other_ptr is null" << endl;
    }
}

void test2() {
    //    unique_ptr可以指向一个数组
    std::unique_ptr<int[]> ptr(new int[10]);
    ptr[9] = 9;

    //    std::shared_ptr<int []> ptr2(new int[10]);  // 这个是不合法的

    //    unique_ptr指定删除器和shared_ptr有区别
    std::shared_ptr<int> ptr3(new int(1), [](int *p) { delete p; }); // 正确
    //    std::unique_ptr<int> ptr4(new int(1), [](int *p){delete  p;}); // 错误

    std::unique_ptr<int, void (*)(int *)> ptr5(new int(1), [](int *p) { delete p; }); // 正确

    std::unique_ptr<int, void (*)(int *)> ptr6(new int(1), [](int *p) { delete p; }); // 正确
    cout << "main finish!" << endl;
}

int main() {
    test1();
//    test2();
    cout << "main finish!" << endl;
    return 0;
}
