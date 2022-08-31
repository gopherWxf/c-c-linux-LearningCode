//1-1-shared_from_this2
#include <iostream>
#include <memory>

using namespace std;

class A : public std::enable_shared_from_this<A> {
public:
    shared_ptr<A> GetSelf() {
        return shared_from_this(); //
    }

    A() {
        cout << "Construction A" << endl;
    }

    ~A() {
        cout << "Destruction A" << endl;
    }
};

int main() {
    shared_ptr<A> sp1(new A);
    shared_ptr<A> sp2 = sp1->GetSelf();  // ok
    cout << "sp1.use_count() = " << sp1.use_count() << endl;
    cout << "sp2.use_count() = " << sp2.use_count() << endl;
    cout << "leave {}" << endl;

    return 0;
}
