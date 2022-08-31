#include <iostream>
#include <memory>

using namespace std;

void test(shared_ptr<int> sp) {
    // sp在test里面的作用域
    cout << "test sp.use_count()" << sp.use_count() << endl;
}

int main2() {
    auto sp1 = make_shared<int>(100); // 优先使用make_shared来构造智能指针
    //相当于
    shared_ptr<int> sp2(new int(100));

//    std::shared_ptr<int> p = new int(1);  // 不能将一个原始指针直接赋值给一个智能指针
//      int *p = new int(1);
    std::shared_ptr<int> p1;
    p1.reset(new int(1));  // 分配资源。
    if (p1) {
        cout << "p1 ptr new int(1) \n";
    }
//    p1.reset();
//    if(!p1) {
//        cout << "p1 ptr NULL \n";
//    }
    std::shared_ptr<int> p2 = p1;

//    // 引用计数此时应该是2
    cout << "p2.use_count() = " << p2.use_count() << endl;  // 2
    p1.reset();   // 释放资源

//    cout << "p1.reset()\n";
//    // 引用计数此时应该是1
    cout << "p2.use_count()= " << p2.use_count() << endl;  // 1
    if (!p1) {  // p1是空的  true false
        cout << "p1 is empty\n";
    }
    if (!p2) { // p2非空
        cout << "p2 is empty\n";
    }

    //裸指针
    int *p = nullptr;
    p2.reset();
    cout << "p2.reset()\n";
    cout << "p2.use_count()= " << p2.use_count() << endl;
    if (!p2) {
        cout << "p2 is empty\n";
    }

    shared_ptr<int> sp5(new int(100));
    test(sp5);
    cout << "sp5.use_count()" << sp5.use_count() << endl;
    return 0;
}

int main() {
    {
        int *ptr = new int;
        shared_ptr<int> p1(ptr);
        shared_ptr<int> p2(ptr); // 逻辑错误
        cout << "p1.use_count()= " << p1.use_count() << endl;
        cout << "p2.use_count()= " << p2.use_count() << endl;
    }
    cout << "main end\n";
}







