#include <iostream>
#include <memory>

using namespace std;
std::weak_ptr<int> gw;




void test1() {
    cout << "---- test1 ------------------" << endl;
    weak_ptr<int> wp;
    {
        shared_ptr<int> sp(new int(1));  //sp.use_count()==1
        wp = sp;                            //wp不会改变引用计数，所以sp.use_count()==1
        shared_ptr<int> sp_ok = wp.lock(); //wp没有重载->操作符。只能这样取所指向的对象
    }
    shared_ptr<int> sp_null = wp.lock(); //sp_null .use_count()==0;
    if (wp.expired()) {
        cout << "shared_ptr is destroy" << endl;
    }
    else {
        cout << "shared_ptr no destroy" << endl;
    }
}

void test2() {
    cout << "---- test2 ------------------" << endl;
    weak_ptr<int> wp;
    shared_ptr<int> sp_ok;
    {
        shared_ptr<int> sp(new int(1));  //sp.use_count()==1
        wp = sp; //wp不会改变引用计数，所以sp.use_count()==1
        sp_ok = wp.lock(); //wp没有重载->操作符。只能这样取所指向的对象
    }

    if (wp.expired()) {
        cout << "shared_ptr is destroy" << endl;
    }
    else {
        cout << "shared_ptr no destroy" << endl;
    }
}

int main() {
    test1();
    test2();
    return 0;
}
void fn(shared_ptr<A>sp) {
    ...
    if(..){
        sp = other_sp;
    } else {
        sp = other_sp2;
    }
}