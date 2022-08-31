//1-1-shared_from_this
#include <iostream>
#include <memory>

using namespace std;

class A {
public:
    shared_ptr<A> GetSelf() {
        return shared_ptr<A>(this); // 不要这么做
    }

    A() {
        cout << "Construction A" << endl;
    }

    ~A() {
        cout << "Destruction A: " << this << endl;
    }
};

int main() {
    shared_ptr<A> sp1(new A);
    shared_ptr<A> sp2 = sp1->GetSelf();
    cout << "sp1.use_count() = " << sp1.use_count() << endl;
    cout << "sp2.use_count() = " << sp2.use_count() << endl;
    return 0;
}
